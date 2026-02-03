#pragma once
#include <mutex>
#include <memory>
#include <condition_variable>


/*
    - create a node struct and have head and tail and have a dummy node as the head
    - head mutex and tail mutex
    - constructor
    - push
    - try_pop (sp & ref)
*/
template <typename T>
class NodeQueue {
    private:
        struct Node {
            std::shared_ptr<T> data;
            std::shared_ptr<Node> next;
            Node() : next(nullptr) {};
            Node(T val) : data(std::make_shared<T>(std::move(val))), next(nullptr) {};
        };

        std::shared_ptr<Node> head;
        Node* tail;
        std::mutex head_mu;
        std::mutex tail_mu;
        std::condition_variable cv;

        Node* get_tail() {
            std::lock_guard<std::mutex> lk(tail_mu);
            return tail;
        }

        std::unique_lock<std::mutex> lock_head() {
            std::unique_lock<std::mutex> hlk(head_mu);
            cv.wait(hlk, [this] { return head.get() != get_tail(); });
            return hlk;
        }

        std::shared_ptr<T> pop_head() {
            std::unique_lock<std::mutex> hlk = lock_head();
            std::shared_ptr<Node> nxt = head.get()->next;
            std::shared_ptr<T> data = head.get()->data;
            head = nxt;
            return data;
        }

    public:
        NodeQueue() : head(std::make_shared<Node>()), tail(head.get()){};     
        
        void push(T value) {
            std::shared_ptr<Node> nxt = std::make_shared<Node>();
            std::lock_guard<std::mutex> lk(tail_mu);
            tail->data = std::make_shared<T>(std::move(value));
            tail->next = nxt;
            tail = nxt.get();
            cv.notify_one();
        };

        bool try_pop(T& value) {
            std::unique_lock<std::mutex> hlk(head_mu);
            if (head.get() == get_tail()) {
                return false;
            }
            std::shared_ptr<Node> nxt = head.get()->next;
            value = std::move(*(head->data));
            head = nxt;
            return true;
        };

        std::shared_ptr<T> try_pop() {
            std::unique_lock<std::mutex> hlk(head_mu);
            if (head.get() == get_tail()) {
                return nullptr;
            }
            std::shared_ptr<Node> nxt = head.get()->next;
            std::shared_ptr<T> res = head->data;
            head = nxt;
            return res;
        };

        void wait_and_pop(T& value) {
           std::shared_ptr<T> data = pop_head();
            value = std::move(*data);
        };

        std::shared_ptr<T> wait_and_pop() {
           return pop_head();
        };
};