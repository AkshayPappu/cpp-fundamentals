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
class SafeQueue {
    private:
        size_t capacity;
        bool closed;
        std::queue<T> q;
        std::mutex mu;
        std::condition_variable cv_not_full;
        std::condition_variable cv_not_empty;
    public:
        SafeQueue(size_t capacity) : capacity(capacity), closed(false) {};

        // delete move ops
        SafeQueue(SafeQueue&&) = delete;
        SafeQueue& operator=(SafeQueue&&) = delete;

        // delete copy ops
        SafeQueue(const SafeQueue<T>& rhs) = delete;
        SafeQueue<T>& operator=(const SafeQueue<T>& rhs) = delete;

        ~SafeQueue() {
            close();
        }

        void close() {
            std::unique_lock<std::mutex> lk(mu);
            closed = true;
            lk.unlock();
            cv_not_full.notify_all();
            cv_not_empty.notify_all();
        }

        bool try_push(T obj) {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
            std::unique_lock<std::mutex> lk(mu);
            bool ok = cv_not_full.wait_until(lk, deadline, [this] { return closed || q.size() < capacity; });
            
            if (closed || !ok) return false;
            q.push(std::move(obj));

            lk.unlock();
            cv_not_empty.notify_one();
            return true;
        };

        bool wait_and_push(T obj) {
            std::unique_lock<std::mutex> lk(mu);
            cv_not_full.wait(lk, [this] { return closed || q.size() < capacity; });
            
            if (closed) return false;

            q.push(std::move(obj));

            lk.unlock();
            cv_not_empty.notify_one();
            return true;
        };

        std::shared_ptr<T> try_pop() {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
            std::unique_lock<std::mutex> lk(mu);
            bool ok = cv_not_empty.wait_until(lk, deadline, [this] { return closed || !q.empty(); });

            if (!ok || (closed && q.empty())) return nullptr;
            std::shared_ptr<T> sp = std::make_shared<T>(std::move(q.front()));
            q.pop();

            lk.unlock();
            cv_not_full.notify_one();
            return sp;
        };

        bool try_pop(T& value) {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
            std::unique_lock<std::mutex> lk(mu);
            bool ok = cv_not_empty.wait_until(lk, deadline, [this] { return closed || !q.empty(); });

            if (!ok || (closed && q.empty())) return false;
            value = std::move(q.front());
            q.pop();
            
            lk.unlock();
            cv_not_full.notify_one();
            return true;
        }

        bool wait_and_pop(T& value) {
            std::unique_lock<std::mutex> lk(mu);
            cv_not_empty.wait(lk, [this] { return closed || !q.empty(); });

            if ((closed && q.empty())) return false;
            value = std::move(q.front());
            q.pop();
            
            lk.unlock();
            cv_not_full.notify_one();
            return true;
        }

        bool empty() {
            std::unique_lock<std::mutex> lk(mu);
            return q.empty();
        }
};