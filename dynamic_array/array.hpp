#pragma once
#include <cstddef>
#include <new>
#include <stdexcept>
#include <utility>

constexpr size_t DEFAULT_CAPACITY = 1;

template <typename T>
class DynamicArray {
    private:
        size_t capacity_;
        size_t size_;
        T* data_;

        T* allocate_raw(size_t n) {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }

        void deallocate_raw(T* p) {
            ::operator delete(p);
        }

        void destroy_range(T* p, size_t n) noexcept {
            for (size_t i = 0; i < n; ++i) {
                (p + i)->~T();
            }
        }

        void grow() {
            size_t new_capacity_ = capacity_ * 2;
            T* new_data_ = allocate_raw(new_capacity_);

            for (size_t i = 0; i < size_; ++i) {
                ::new (static_cast<void*>(new_data_ + i)) T(std::move_if_noexcept((data_[i])));
            }

            destroy_range(data_, size_);
            deallocate_raw(data_);

            data_ = new_data_;
            capacity_ = new_capacity_;
        }

    public:
        DynamicArray() : capacity_(DEFAULT_CAPACITY), size_(0), data_(allocate_raw(DEFAULT_CAPACITY)) {};

        explicit DynamicArray(size_t capacity) : capacity_(capacity), size_(0), data_(allocate_raw(capacity_)) {};

        void push_back(const T& value) {
            if (size_ == capacity_) grow();
            ::new (static_cast<void*>(data_ + size_)) T(value);
            ++size_;
        }

        void push_back(T&& value) {
            if (size_ == capacity_) grow();
            ::new (static_cast<void*>(data_ + size_)) T(std::move(value));
            ++size_;
        }

        template<typename... Args>
        void emplace_back(Args&&... args) {
            if (size_ == capacity_) grow();
            ::new (static_cast<void*>(data_ + size_)) T(std::forward<Args>(args)...);
            ++size_;
        }

        void pop_back() {
            if (size_ == 0) return;
            (data_ + size_ - 1)->~T();
            --size_;
        }

        T& operator[](size_t index) {
            if (index >= size_) throw std::out_of_range("index is out of range");
            return data_[index];

        }
        const T& operator[](size_t index) const {
            if (index >= size_) throw std::out_of_range("index is out of range");
            return data_[index];
        }

        size_t size() const noexcept { return size_; }
        size_t capacity() const noexcept { return capacity_; }
        bool empty() const noexcept { return size_ == 0; }
};