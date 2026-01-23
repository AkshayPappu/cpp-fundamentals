#pragma once
#include <queue>
#include <mutex>
#include <memory>

template <typename T>
class SafeQueue {
    private:
        size_t capacity;
        std::queue<T> q;
        std::mutex mu;
    public:
        SafeQueue(size_t capacity);
        SafeQueue(SafeQueue&& rhs) noexcept;
        SafeQueue<T>& operator=(SafeQueue<T>&& rhs) noexcept;

        // delete copy ops
        SafeQueue(const SafeQueue<T>& rhs) = delete;
        SafeQueue<T>& operator=(const SafeQueue<T>& rhs) = delete;

        ~SafeQueue();

        void push(T obj);
        std::shared_ptr<T> pop();
};