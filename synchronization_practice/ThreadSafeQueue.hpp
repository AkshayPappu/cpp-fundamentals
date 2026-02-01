#pragma once
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <chrono>

/*
    Updates:
    - use condition variables for push and pop operations
    - have condition variable wait_until to avoid wasting cycles
    
*/

template <typename T>
class ThreadSafeQueue {
    private:
        size_t capacity;
        std::queue<T> q;
        std::mutex mu;
        std::condition_variable cv_not_full;
        std::condition_variable cv_not_empty;
    public:
        ThreadSafeQueue(size_t capacity) : capacity(capacity) {};

        ThreadSafeQueue(ThreadSafeQueue&& rhs) noexcept {
            std::lock_guard<std::mutex> lk(rhs.mu);
            capacity = rhs.capacity;
            q = std::move(rhs.q);
        };

        ThreadSafeQueue<T>& operator=(ThreadSafeQueue<T>&& rhs) noexcept {
            std::scoped_lock lk(mu, rhs.mu);
            if (this == &rhs) return *this;
            capacity = rhs.capacity;
            q = std::move(rhs.q);
            return *this;
        };

        // delete copy ops
        ThreadSafeQueue(const ThreadSafeQueue<T>& rhs) = delete;
        ThreadSafeQueue<T>& operator=(const ThreadSafeQueue<T>& rhs) = delete;

        ~ThreadSafeQueue() = default;

        bool push(T obj) {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
            std::unique_lock<std::mutex> lk(mu);
            bool ok = cv_not_full.wait_until(lk, deadline, [this] { return q.size() < capacity; });
            
            if (!ok) return false;
            q.push(std::move(obj));

            lk.unlock();
            cv_not_empty.notify_one();
            return true;
        };

        std::shared_ptr<T> pop() {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
            std::unique_lock<std::mutex> lk(mu);
            bool ok = cv_not_empty.wait_until(lk, deadline, [this] { return !q.empty(); });

            if (!ok) return nullptr;
            std::shared_ptr<T> sp = std::make_shared<T>(std::move(q.front()));
            q.pop();

            lk.unlock();
            cv_not_full.notify_one();
            return sp;
        };
};