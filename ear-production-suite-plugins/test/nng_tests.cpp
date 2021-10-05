#include <catch2/catch_all.hpp>
#include "nng-cpp/nng.hpp"
#include "nng-cpp/asyncio.hpp"
#include <atomic>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

// We have to define different addresses for each test
// otherwise we set up a race condition in which we can try to
// listen on an address before it has been properly released
int addressSuffix = 0;

std::string makeAddress() {
  std::string addr = "tcp://localhost:444";
  addr.append(std::to_string(addressSuffix++));
  return addr;
}

std::string as_string(const nng::Buffer& buffer) {
  return std::string{(const char*)buffer.data(), buffer.size()};
}

std::string as_string(const nng::Message& msg) {
  return std::string{(const char*)msg.data(), msg.size()};
}

using Catch::Matchers::Contains;

TEST_CASE("sync rep-req") {
  nng::ReqSocket requestSocket;
  nng::RepSocket replySocket;
  requestSocket.setOpt(nng::options::RecvTimeout, 5000ms);
  replySocket.setOpt(nng::options::RecvTimeout, 5000ms);
  requestSocket.setOpt(nng::options::SendTimeout, 5000ms);
  replySocket.setOpt(nng::options::SendTimeout, 5000ms);

  auto addr = makeAddress();
  replySocket.listen(addr.c_str());
  requestSocket.dial(addr.c_str());

  SECTION("basic use case") {
    std::string msg = "hello world!";
    requestSocket.send(msg);
    {
      auto buffer = replySocket.read();
      REQUIRE(as_string(buffer) == msg);
      replySocket.send(buffer);
    }
    auto buffer = requestSocket.read();
    REQUIRE(as_string(buffer) == msg);
  }

  SECTION("failing reply-without-request") {
    std::string msg = "reponse";
    REQUIRE_THROWS(replySocket.send(msg));
  }

  SECTION("second request discards response") {
    std::string msg1 = "reponse1";
    std::string msg2 = "reponse2";
    requestSocket.send(msg1);
    requestSocket.send(msg2);

    {
      auto buffer = replySocket.read();
      replySocket.send(buffer);
    }
    {
      auto buffer = replySocket.read();
      replySocket.send(buffer);
    }
    auto reply = requestSocket.read();
    REQUIRE(as_string(reply) == msg2);
    REQUIRE_THROWS(requestSocket.read());
  }
}

TEST_CASE("async rep-req") {
  nng::ReqSocket requestSocket;
  nng::RepSocket replySocket;

  requestSocket.setOpt(nng::options::RecvTimeout, 5000ms);
  replySocket.setOpt(nng::options::RecvTimeout, 5000ms);
  requestSocket.setOpt(nng::options::SendTimeout, 5000ms);
  replySocket.setOpt(nng::options::SendTimeout, 5000ms);

  auto addr = makeAddress();
  replySocket.listen(addr.c_str());
  requestSocket.dial(addr.c_str());

  // Note: testing sending/receiveing separately so we always have
  // at least one blocking call left to ensure some checkpoints in our unit
  // tests.

  SECTION("sending") {
    std::string msg = "Hello, World!";
    bool callbackTriggered = false;
    requestSocket.asyncSend(msg, [&callbackTriggered](const std::error_code& ec,
                                                      nng::Message message) {
      REQUIRE(!ec);
      callbackTriggered = true;
    });
    auto buffer = replySocket.read();
    requestSocket.asyncWait();
    REQUIRE(callbackTriggered == true);
    std::string req = as_string(buffer);
    REQUIRE(req.size() == msg.size());
    REQUIRE(req == msg);
    replySocket.send(buffer);
    auto reply = requestSocket.read();
    REQUIRE(as_string(reply) == msg);
  }

  SECTION("receiving") {
    std::string msg = "Hello, World!";

    requestSocket.send(msg);
    replySocket.asyncRead(
        [&replySocket](std::error_code ec, const nng::Message& msg) {
          REQUIRE(!ec);
          REQUIRE(msg.isValid());
          replySocket.send(msg);
        });
    auto reply = requestSocket.read();
    REQUIRE(as_string(reply) == msg);
  }
}

TEST_CASE("async sleep") {
  nng::AsyncIO aio;
  bool callbackCalled = false;
  aio.sleep(std::chrono::milliseconds{10},
            [&callbackCalled](std::error_code ec) { callbackCalled = true; });
  aio.wait();
  REQUIRE(callbackCalled);
}

TEST_CASE("pub-sub") {
  nng::PubSocket pubSocket;
  nng::SubSocket subSocket1;
  nng::SubSocket subSocket2;

  subSocket1.setOpt(nng::options::RecvTimeout, 5000ms);
  subSocket2.setOpt(nng::options::RecvTimeout, 5000ms);
  pubSocket.setOpt(nng::options::SendTimeout, 5000ms);
  std::atomic<unsigned int> receiveIndicator{0};

  auto addr = makeAddress();
  pubSocket.listen(addr.c_str());
  subSocket1.dial(addr.c_str());
  // tests the "string" set option version
  subSocket1.setOpt(nng::options::SubSubscribe, "", 0);
  subSocket2.dial(addr.c_str());
  // tests the "ptr + size " set option version
  subSocket2.setOpt(nng::options::SubSubscribe, "", 0);
  subSocket1.asyncRead(
      [&receiveIndicator](std::error_code ec, nng::Message msg) {
        if (!ec) {
          ++receiveIndicator;
        }
      });
  subSocket2.asyncRead(
      [&receiveIndicator](std::error_code ec, nng::Message msg) {
        if (!ec) {
          receiveIndicator += 2;
        }
      });

  // add some additional delay before going to sending/receiving
  // seems that the pub/sub socket might not be ready immediatly,
  // which should not be a problem in real-world applications
  // but within this tight testing function.
  std::this_thread::sleep_for(20ms);

  SECTION("Receive all messages") {
    pubSocket.send(std::string{"Hello, World"});
    subSocket1.asyncWait();
    subSocket2.asyncWait();
    REQUIRE(receiveIndicator == 3);
  }
  SECTION("Re-subscribe to some submessages") {
    subSocket2.setOpt(nng::options::SubUnsubscribe, "", 0);
    subSocket1.setOpt(nng::options::SubUnsubscribe, "", 0);
    subSocket2.setOpt(nng::options::SubSubscribe, "Hello", 5);
    subSocket1.setOpt(nng::options::SubSubscribe, "Else", 4);
    pubSocket.send(std::string{"Hello, World"});
    subSocket2.asyncWait();
    subSocket1.asyncStop();
    REQUIRE(receiveIndicator == 2);
  }
}

TEST_CASE("dialer") {
  nng::ReqSocket requestSocket;
  nng::RepSocket replySocket;
  requestSocket.setOpt(nng::options::RecvTimeout, 5000ms);
  replySocket.setOpt(nng::options::RecvTimeout, 5000ms);
  requestSocket.setOpt(nng::options::SendTimeout, 5000ms);
  replySocket.setOpt(nng::options::SendTimeout, 5000ms);

  nng::Dialer dialer;
  auto addr = makeAddress();
  replySocket.listen(addr.c_str());

  SECTION("basic use case") {
    dialer = requestSocket.createDialer(addr.c_str());
    REQUIRE(dialer.id() >= 0);
    dialer.start();
    std::string msg = "hello world!";
    requestSocket.send(msg);
    {
      auto buffer = replySocket.read();
      REQUIRE(as_string(buffer) == msg);
      replySocket.send(buffer);
    }
    auto buffer = requestSocket.read();
    REQUIRE(as_string(buffer) == msg);
    dialer.close();
  }

  SECTION("uninitialized dialers") {
    // id can be used to check if valid
    REQUIRE(dialer.id() == -1);
    // should do nothing, i.e. it's safe to close an uninitialized dialer
    REQUIRE_NOTHROW(dialer.close());
    // starting an invalid dialer, on the other hand, is not valid
    REQUIRE_THROWS(dialer.start());
  }
}
