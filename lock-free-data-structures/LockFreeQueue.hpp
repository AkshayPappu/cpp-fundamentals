#pragma once
#include <atomic>
#include <vector>
#include <memory>

template <typename T>
class LockFreeQueue {
    private:
        size_t capacity;
        std::vector<T> q;
        std::atomic<int> head;
        std::atomic<int> tail;
    public:
        LockFreeQueue(size_t cap) : capacity(cap), q(capacity), head(0), tail(0) {};

        bool try_push(T value) {
            int h = head.load();
            int t = tail.load();
            int next = (t + 1) % capacity;
            if (h == next) return false;
            q[t] = std::move(value);
            tail.store((t + 1) % capacity);
            return true;
        }

        std::unique_ptr<T> try_pop() {
            int h = head.load();
            int t = tail.load();
            if (h == t) return std::unique_ptr<T>();
            std::unique_ptr<T> res = std::make_unique<T>(std::move(q[h]));
            head.store((h + 1) % capacity);
            return std::move(res);
        }

        bool try_pop(T& value) {
            int h = head.load();
            int t = tail.load();
            if (h == t) return false;
            value = std::move(q[h]);
            head.store((h + 1) % capacity);
            return true;
        }

        bool empty() {
            return tail.load() == head.load();
        }

        bool full() {
            return (tail.load() + 1) % capacity == head.load();
        }
};