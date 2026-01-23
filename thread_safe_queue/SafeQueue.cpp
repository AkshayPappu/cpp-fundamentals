#include "SafeQueue.hpp"

template <typename T>
SafeQueue<T>::SafeQueue(size_t capacity) : capacity(capacity) {};

template <typename T>
SafeQueue<T>::SafeQueue(SafeQueue&& rhs) noexcept {
    std::lock_guard<std::mutex> lk(rhs.mu);
    capacity = rhs.capacity;
    q = std::move(rhs.q);
}

template <typename T>
SafeQueue<T>& SafeQueue<T>::operator=(SafeQueue<T>&& rhs) noexcept {
    std::scoped_lock lk(mu, rhs.mu);
    capacity = rhs.capacity;
    q = std::move(rhs.q);
    return *this;
}

template <typename T>
SafeQueue<T>::~SafeQueue() = default;

template <typename T>
void SafeQueue<T>::push(T obj) {
    std::lock_guard<std::mutex> lk(mu);
    if (q.size() >= capacity) return;
    q.push(std::move(obj));
}

template <typename T>
std::shared_ptr<T> SafeQueue<T>::pop() {
    std::lock_guard<std::mutex> lk(mu);
    if (q.size() == 0) return nullptr;

    std::shared_ptr<T> sp = std::make_shared<T>(q.front());
    q.pop();
    return sp;
}

// Explicit instantiations
template class SafeQueue<int>;