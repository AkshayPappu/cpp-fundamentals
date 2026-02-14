#pragma once
#include <atomic>
#include <vector>
#include <memory>

template <typename T>
class LockFreeQueue {
    private:
        size_t capacity;
        std::vector<T> q;
        std::atomic<size_t> head;
        std::atomic<size_t> tail;
    public:
        LockFreeQueue(size_t cap) : capacity(cap), q(capacity), head(0), tail(0) {};
        
        bool try_push(T value) {
            size_t h = head.load(std::memory_order_relaxed);
            size_t t = tail.load(std::memory_order_acquire);
            size_t next = (t + 1) % capacity;
            if (next == h) return false;
            q[t] = std::move(value);
            tail.store((t + 1) % capacity, std::memory_order_release);
            return true;
        }

        bool try_pop(T& value) {
            size_t h = head.load(std::memory_order_relaxed);
            size_t t = tail.load(std::memory_order_acquire);
            if (h == t) return false;
            value = std::move(q[h]);
            head.store((h + 1) % capacity, std::memory_order_release);
            return true;
        }

        std::unique_ptr<T> try_pop() {
            size_t h = head.load(std::memory_order_relaxed);
            size_t t = tail.load(std::memory_order_acquire);
            if (h == t) return {};
            std::unique_ptr<T> res = std::make_unique<T>(std::move(q[h]));
            head.store((h + 1) % capacity, std::memory_order_release);
            return std::move(res);
        }

        bool full() {
            size_t h = head.load(std::memory_order_relaxed);
            size_t t = tail.load(std::memory_order_relaxed);
            return (t + 1) % capacity == h;
        }

        bool empty() {
            size_t h = head.load(std::memory_order_relaxed);
            size_t t = tail.load(std::memory_order_relaxed);
            return h == t;
        }
};