#pragma once
#include <atomic>
#include <memory>

template <typename T>
class LockFreeStack {
    private:
        struct Node {
            std::unique_ptr<T> data;
            Node* next;
            Node(T val) : data(std::make_unique<T>(val)), next(nullptr) {};
        };

        std::atomic<Node*> head;
    public:
        LockFreeStack() : head(nullptr) {};
        
        void push(T value) {
            Node* new_head = new Node(std::move(value));
            Node* old_head = head.load(std::memory_order_acquire);
            new_head->next = old_head;
            while (!head.compare_exchange_weak(new_head->next, new_head, std::memory_order_acq_rel, std::memory_order_relaxed));
        }

        std::shared_ptr<T> try_pop() {
            Node* old_head = head.load(std::memory_order_acquire);
            while (old_head && !head.compare_exchange_weak(old_head, old_head->next, std::memory_order_acq_rel, std::memory_order_relaxed));
            if (!old_head) return {};
            return std::move(old_head->data);
        }

        bool try_pop(T& value) {
            Node* old_head = head.load(std::memory_order_acquire);
            while(old_head && !head.compare_exchange_weak(old_head, old_head->next, std::memory_order_acq_rel, std::memory_order_relaxed));
            if (!old_head) return false;
            value = std::move(*(old_head->data));
            return true;
        }
};