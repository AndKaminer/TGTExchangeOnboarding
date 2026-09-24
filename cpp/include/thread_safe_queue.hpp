#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace thread_safe_queue {

template<typename T>
concept CopyQueueable = std::copy_constructible<T> && std::destructible<T>;

template <CopyQueueable queueable>
class ThreadSafeQueue {
public:
  queueable pop() {
    std::scoped_lock(mtx);
    cv.wait(mtx, [this]{ return q.size() > 0; });
    return q.pop();
  }

  void push(queueable arg) {
    {
      std::scoped_lock(mtx);
      q.push(arg);
    }

    cv.notify_all();
  }

private:
  std::queue<queueable> q;
  std::mutex mtx;
  std::condition_variable cv;
};

};
