#pragma once
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace BTCore {

template <typename T> class mutexQueue {
  std::queue<T> m_q;
  std::mutex m_mutex;
  std::condition_variable m_cv;

public:
  void push(const T &x) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_q.push(x);
    }
    m_cv.notify_one();
  }
  void push(T &&x) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_q.push(std::move(x));
    }
    m_cv.notify_one();
  }
  template <typename... Args> void emplace(Args &&...args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_q.emplace(std::forward<Args>(args)...);
    m_cv.notify_one();
  }

  bool tryPop(T &value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_q.empty())
      return false;

    value = std::move(m_q.front());
    m_q.pop();
    return true;
  }

  void waitAndPop(T &value) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this] { return !m_q.empty(); });
    value = std::move(m_q.front());
    m_q.pop();
  }

  bool waitAndPop(T &value, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(m_mutex);

    bool hasItem =
        m_cv.wait_for(lock, timeout, [this] { return !m_q.empty(); });

    if (hasItem) {
      value = std::move(m_q.front());
      m_q.pop();
      return true;
    }
    return false;
  }

  bool isEmpty() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_q.empty();
  }
};
} // namespace BTCore
