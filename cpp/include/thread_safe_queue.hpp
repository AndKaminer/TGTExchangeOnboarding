#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <stop_token>

namespace thread_safe_queue {

template<typename T>
concept CopyQueueable = std::copy_constructible<T> && std::destructible<T>;

template <CopyQueueable queueable>
class ThreadSafeQueue {
public:
  ThreadSafeQueue() = default;
  std::optional<queueable> pop(std::stop_token stoken) {
    std::unique_lock<std::mutex> lock (mtx);
    if (!cv.wait(lock, stoken, [this]{ return !q.empty(); })) {
      return std::nullopt;
    }
    queueable out {q.front()};
    q.pop();
    return out;
  }

  void push(queueable arg) {
    {
      std::unique_lock<std::mutex> lock{mtx};
      q.push(arg);
    }

    cv.notify_all();
  }

private:
  std::queue<queueable> q;
  std::mutex mtx;
  std::condition_variable_any cv;
};

};
