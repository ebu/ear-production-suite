#include <catch2/catch_all.hpp>

#include "communication/commands.hpp"
#include "communication/input_control_connection.hpp"
#include "communication/monitoring_control_connection.hpp"
#include "communication/scene_connection_manager.hpp"
#include "nng-cpp/nng.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

namespace {

using ear::plugin::communication::ConnectionId;
using ear::plugin::communication::RequestVariant;
using ear::plugin::communication::SceneConnectionManager;

std::atomic<unsigned int> endpointCounter{0};

std::string makeEndpoint() {
  return "inproc://eps-control-connection-test-" +
         std::to_string(endpointCounter.fetch_add(1));
}

class EventLatch {
 public:
  void signal() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      signaled_ = true;
    }
    condition_.notify_all();
  }

  bool waitFor(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return condition_.wait_for(lock, timeout,
                               [this] { return signaled_; });
  }

  void reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    signaled_ = false;
  }

 private:
  std::condition_variable condition_;
  std::mutex mutex_;
  bool signaled_{false};
};

class FakeSceneMaster {
 public:
  using RequestHandler =
      std::function<void(FakeSceneMaster&, nng::RepSocket&,
                         const RequestVariant&, std::size_t)>;

  FakeSceneMaster(std::string endpoint, RequestHandler handler)
      : endpoint_(std::move(endpoint)), handler_(std::move(handler)) {
    thread_ = std::thread(&FakeSceneMaster::run, this);
  }

  ~FakeSceneMaster() { stop(); }

  FakeSceneMaster(const FakeSceneMaster&) = delete;
  FakeSceneMaster& operator=(const FakeSceneMaster&) = delete;

  void stop() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopRequested_ = true;
      firstRequestReleased_ = true;
      recycleRequested_ = true;
    }
    condition_.notify_all();
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  const std::string& endpoint() const { return endpoint_; }

  bool waitUntilReady(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return condition_.wait_for(lock, timeout,
                               [this] { return ready_; });
  }

  bool waitForRequestCount(std::size_t count,
                           std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return condition_.wait_for(lock, timeout, [this, count] {
      return requests_.size() >= count || serverError_;
    });
  }

  RequestVariant requestAt(std::size_t index) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return requests_.at(index);
  }

  void releaseFirstRequest() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      firstRequestReleased_ = true;
    }
    condition_.notify_all();
  }

  bool waitForFirstRequestRelease(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return condition_.wait_for(lock, timeout, [this] {
      return firstRequestReleased_ || stopRequested_;
    });
  }

  void recycleConnection() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      recycleRequested_ = true;
      firstRequestReleased_ = true;
    }
    condition_.notify_all();
  }

  bool waitForRecycle(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return condition_.wait_for(lock, timeout, [this] {
      return recycled_ || serverError_;
    });
  }

  bool hasServerError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return serverError_;
  }

 private:
  std::unique_ptr<nng::RepSocket> createSocket() {
    auto socket = std::make_unique<nng::RepSocket>();
    socket->setOpt(nng::options::RecvTimeout, 20ms);
    socket->setOpt(nng::options::SendTimeout, 100ms);
    socket->listen(endpoint_.c_str());
    return socket;
  }

  bool isStopping() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stopRequested_;
  }

  bool shouldRecycle() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return recycleRequested_ && !recycled_;
  }

  void recordServerError() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      serverError_ = true;
    }
    condition_.notify_all();
  }

  void run() {
    try {
      auto socket = createSocket();
      {
        std::lock_guard<std::mutex> lock(mutex_);
        ready_ = true;
      }
      condition_.notify_all();

      while (!isStopping()) {
        if (shouldRecycle()) {
          socket.reset();
          std::this_thread::sleep_for(100ms);
          if (isStopping()) {
            break;
          }
          socket = createSocket();
          {
            std::lock_guard<std::mutex> lock(mutex_);
            recycled_ = true;
          }
          condition_.notify_all();
        }

        try {
          auto buffer = socket->read();
          auto request = ear::plugin::communication::parseRequest(buffer);
          std::size_t requestCount;
          {
            std::lock_guard<std::mutex> lock(mutex_);
            requests_.push_back(request);
            requestCount = requests_.size();
          }
          condition_.notify_all();
          handler_(*this, *socket, request, requestCount);
        } catch (const std::system_error& error) {
          if (error.code().value() != NNG_ETIMEDOUT && !isStopping()) {
            recordServerError();
          }
        }
      }
    } catch (const std::exception&) {
      recordServerError();
      {
        std::lock_guard<std::mutex> lock(mutex_);
        ready_ = true;
      }
      condition_.notify_all();
    }
  }

  std::string endpoint_;
  RequestHandler handler_;
  std::thread thread_;
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  std::vector<RequestVariant> requests_;
  bool ready_{false};
  bool recycled_{false};
  bool recycleRequested_{false};
  bool firstRequestReleased_{false};
  bool stopRequested_{false};
  bool serverError_{false};
};

void sendManagerResponse(nng::RepSocket& socket,
                         SceneConnectionManager& manager,
                         const RequestVariant& request) {
  try {
    auto response = manager.handle(
        ear::plugin::communication::Request{request});
    auto buffer = ear::plugin::communication::serialize(response);
    socket.send(buffer);
  } catch (const std::system_error&) {
    // A response to a cancelled request may arrive after the client has
    // already discarded the pipe. The following request is the assertion
    // that matters in these tests.
  }
}

TEST_CASE("input control keeps pipe callbacks non-blocking across reconnect") {
  SceneConnectionManager manager;
  EventLatch established;
  EventLatch lost;
  ear::plugin::communication::InputControlConnection connection(nullptr);
  FakeSceneMaster master(
      makeEndpoint(),
      [&manager](FakeSceneMaster& master, nng::RepSocket& socket,
                 const RequestVariant& request, std::size_t requestCount) {
        if (requestCount == 1) {
          master.waitForFirstRequestRelease(2s);
          return;
        }
        sendManagerResponse(socket, manager, request);
      });
  REQUIRE(master.waitUntilReady(1s));

  std::atomic<int> establishedCount{0};
  connection.onConnectionEstablished(
      [&established, &establishedCount](ConnectionId, std::string) {
        ++establishedCount;
        established.signal();
      });
  connection.onConnectionLost([&lost] { lost.signal(); });
  connection.start(master.endpoint());

  REQUIRE(master.waitForRequestCount(1, 1s));
  master.recycleConnection();
  REQUIRE(master.waitForRecycle(1s));

  // The pipe callback must be able to cancel the pending handshake instead
  // of remaining blocked in a synchronous receive.
  REQUIRE(lost.waitFor(750ms));
  REQUIRE(master.waitForRequestCount(2, 2s));
  REQUIRE(established.waitFor(2s));
  REQUIRE(establishedCount == 1);
  REQUIRE_FALSE(master.hasServerError());

  lost.reset();
  master.stop();
  REQUIRE(lost.waitFor(1s));
}

TEST_CASE(
    "monitoring control keeps pipe callbacks non-blocking across reconnect") {
  SceneConnectionManager manager;
  EventLatch established;
  EventLatch lost;
  ear::plugin::communication::MonitoringControlConnection connection;
  FakeSceneMaster master(
      makeEndpoint(),
      [&manager](FakeSceneMaster& master, nng::RepSocket& socket,
                 const RequestVariant& request, std::size_t requestCount) {
        if (requestCount == 1) {
          master.waitForFirstRequestRelease(2s);
          return;
        }
        sendManagerResponse(socket, manager, request);
      });
  REQUIRE(master.waitUntilReady(1s));

  std::atomic<int> establishedCount{0};
  connection.onConnectionEstablished(
      [&established, &establishedCount](ConnectionId, std::string) {
        ++establishedCount;
        established.signal();
      });
  connection.onConnectionLost([&lost] { lost.signal(); });
  connection.start(master.endpoint());

  REQUIRE(master.waitForRequestCount(1, 1s));
  master.recycleConnection();
  REQUIRE(master.waitForRecycle(1s));

  REQUIRE(lost.waitFor(750ms));
  REQUIRE(master.waitForRequestCount(2, 2s));
  REQUIRE(established.waitFor(2s));
  REQUIRE(establishedCount == 1);
  REQUIRE_FALSE(master.hasServerError());

  lost.reset();
  master.stop();
  REQUIRE(lost.waitFor(1s));
}

TEST_CASE("input control restarts negotiation after an ID change") {
  SceneConnectionManager manager;
  const auto requestedId = ConnectionId::generate();
  EventLatch established;
  EventLatch lost;
  std::atomic<int> establishedCount{0};
  ConnectionId establishedId;
  std::mutex establishedMutex;
  ear::plugin::communication::InputControlConnection connection(nullptr);
  FakeSceneMaster master(
      makeEndpoint(),
      [&manager](FakeSceneMaster& master, nng::RepSocket& socket,
                 const RequestVariant& request, std::size_t requestCount) {
        if (requestCount == 1) {
          master.waitForFirstRequestRelease(2s);
        }
        sendManagerResponse(socket, manager, request);
      });
  REQUIRE(master.waitUntilReady(1s));

  connection.onConnectionEstablished(
      [&established, &establishedId, &establishedMutex,
       &establishedCount](ConnectionId id, std::string) {
        {
          std::lock_guard<std::mutex> lock(establishedMutex);
          establishedId = id;
        }
        ++establishedCount;
        established.signal();
      });
  connection.onConnectionLost([&lost] { lost.signal(); });
  connection.start(master.endpoint());

  REQUIRE(master.waitForRequestCount(1, 1s));
  connection.setConnectionId(requestedId);
  master.releaseFirstRequest();

  REQUIRE(master.waitForRequestCount(2, 2s));
  auto secondRequest = master.requestAt(1);
  REQUIRE(boost::get<ear::plugin::communication::NewConnectionMessage>(
              secondRequest)
              .connectionId() == requestedId);
  REQUIRE(master.waitForRequestCount(3, 2s));
  REQUIRE(established.waitFor(2s));
  REQUIRE(establishedCount == 1);
  {
    std::lock_guard<std::mutex> lock(establishedMutex);
    REQUIRE(establishedId == requestedId);
  }
  REQUIRE_FALSE(master.hasServerError());

  lost.reset();
  master.stop();
  REQUIRE(lost.waitFor(1s));
}

}  // namespace
