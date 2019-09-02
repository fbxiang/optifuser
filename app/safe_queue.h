#pragma once
#include <mutex>
#include <queue>

template <typename T> class SafeQueue {
private:
  std::queue<T> q;
  std::mutex m;

public:
  void push(T elem);
  T pop();
  bool empty();
};

template <typename T> void SafeQueue<T>::push(T elem) {
  m.lock();
  q.push(elem);
  m.unlock();
}

template <typename T> T SafeQueue<T>::pop() {
  m.lock();
  T elem = q.front();
  q.pop();
  m.unlock();
  return elem;
}

template <typename T> bool SafeQueue<T>::empty() {
  m.lock();
  bool e = q.empty();
  m.unlock();
  return e;
}
