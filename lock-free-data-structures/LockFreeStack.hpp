#pragma once
#include <atomic>
#include <memory>

/*
    Note: This does not handle ABA or memory reclamation, but is sufficient for interviews
*/

template <typename T>
class LockFreeStack {
    private:
        struct Node {
            std::unique_ptr<T> data;
            Node* next;
            Node() : data(nullptr), next(nullptr) {};
            Node(T value) : data(std::make_unique<T>(value)), next(nullptr) {};
        };

        std::atomic<Node*> head;
    public:
        LockFreeStack() : head(nullptr) {};


        LockFreeStack(const LockFreeStack& rhs) = delete;
        LockFreeStack& operator=(const LockFreeStack& rhs) = delete;

        void push(T value) {
            Node* new_head = new Node(std::move(value));
            new_head->next = head.load();
            while (!head.compare_exchange_weak(new_head->next, new_head));
        }

        std::unique_ptr<T> try_pop() {
            Node* old_head = head.load();
            while (old_head && !head.compare_exchange_weak(old_head, old_head->next));
            if (!old_head) return std::unique_ptr<T>();
            return std::move(old_head->data);
            
        }

        bool try_pop(T& value) {
            Node* old_head = head.load();
            while (old_head && !head.compare_exchange_weak(old_head, old_head->next));
            if (!old_head) return false;
            value = std::move(*(old_head->data));
            return true;
        }
};