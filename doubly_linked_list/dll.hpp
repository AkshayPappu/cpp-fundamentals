#pragma once
#include <optional>
#include <cstddef>
#include <utility>


template <typename T>
struct Node {
    T value;
    Node<T>* prev;
    Node<T>* next;
    void* owner;

    Node() : value{}, prev(nullptr), next(nullptr), owner(nullptr) {}
    Node(T value) : value(value), prev(nullptr), next(nullptr), owner(nullptr) {}
};

template <typename T>
class DLL {
    private:
        Node<T>* head;
        Node<T>* tail;
        size_t size_;
    public:
        DLL() {
            head = new Node<T>();
            tail = new Node<T>();
            size_ = 0;
            head->next = tail;
            tail->prev = head;
            head->prev = nullptr;
            tail->next = nullptr;
            head->owner = this;
            tail->owner = this;
        }

        ~DLL() noexcept {
            Node<T>* cur = head ? head->next : nullptr;
            while (cur && cur != tail) {
                Node<T>* next = cur->next;
                delete cur;
                cur = next;
            }

            delete head;
            delete tail;
            
            head = nullptr;
            tail = nullptr;
            size_ = 0;
        }

        void push_front(T value) {
            Node<T>* new_node = new Node<T>(value);
            Node<T>* next = head->next;
            
            new_node->owner = this;
            head->next = new_node;
            new_node->prev = head;
            new_node->next = next;
            next->prev = new_node;
            ++size_;
        };

        std::optional<T> pop_front() {
            if (size_ == 0) return std::nullopt;
            Node<T>* node = head->next;
            Node<T>* next = node->next;

            head->next = next;
            next->prev = head;
            node->next = nullptr;
            node->prev = nullptr;
            node->owner = nullptr;

            std::optional<T> out(std::move(node->value));
            delete node;
            --size_;

            return out;
        };

        void push_back(T value) {
            Node<T>* new_node = new Node<T>(value);
            Node<T>* prev = tail->prev;
            
            new_node->owner = this;
            tail->prev = new_node;
            prev->next = new_node;
            new_node->next = tail;
            new_node->prev = prev;
            ++size_;
        };

        std::optional<T> pop_back() {
            if (size_ == 0) return std::nullopt;
            
            Node<T>* node = tail->prev;
            Node<T>* prev = node->prev;

            tail->prev = prev;
            prev->next = tail;
            node->next = nullptr;
            node->prev = nullptr;
            node->owner = nullptr;

            std::optional<T> out(std::move(node->value));
            delete node;
            --size_;

            return out;
        }

        std::optional<T> pop(Node<T>* node) {
            if (!node || node == head || node == tail) return std::nullopt;
            if (node->owner != this) return std::nullopt;
            
            Node<T>* prev = node->prev;
            Node<T>* next = node->next;

            prev->next = next;
            next->prev = prev;
            node->next = nullptr;
            node->prev = nullptr;
            node->owner = nullptr;

            std::optional<T> out(std::move(node->value));
            delete node;
            --size_;

            return out;
        }

        size_t size() const noexcept { return size_; }
};