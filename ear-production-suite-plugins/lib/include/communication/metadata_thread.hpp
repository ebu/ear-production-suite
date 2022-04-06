//
// Created by Richard Bailey on 06/04/2022.
//

#ifndef EAR_PRODUCTION_SUITE_METADATA_THREAD_HPP
#define EAR_PRODUCTION_SUITE_METADATA_THREAD_HPP
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>

namespace ear::plugin {
class MetadataThread {
public:
    using MessageQueue = std::vector<std::function<void()>>;
    MetadataThread() : thread_([this](){updateLoop();}) {
    }
    ~MetadataThread() {
        stop();
    }

    void post(std::function<void()> message) {
        if(run_.load()) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                messages_.push_back(std::move(message));
            }
            condition_.notify_one();
        }
    }


private:
    MessageQueue getMessages() {
        MessageQueue messages;
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this](){return !messages_.empty() || !run_.load();});
        messages = messages_;
        messages_.clear();
        lock.unlock();
        return messages;
    }

    void updateLoop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            messages_.reserve(INITIAL_QUEUE_CAPACITY);
        }
        while(run_.load()) {
            auto messages = getMessages();
            for(auto const& message : messages) {
                message();
            }
        }
    }

    void stop() {
        run_.store(false);
        condition_.notify_one();
        thread_.join();
    }

    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic_bool run_{true};
    std::thread thread_;
    std::vector<std::function<void()>> messages_;
    static const int INITIAL_QUEUE_CAPACITY{64};
};
}


#endif //EAR_PRODUCTION_SUITE_METADATA_THREAD_HPP
