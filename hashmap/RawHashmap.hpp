#pragma once
#include <cstddef>
#include <vector>
#include <new>
#include <functional>
#include <utility>

template <typename T, typename U>
struct Entry {
    T key;
    U value;

    Entry() : key(T()), value(U()) {}
    Entry(T key, U value) : key(key), value(value) {}
    ~Entry() = default;
    Entry(const Entry&) = default;
    Entry(Entry&&) = default;

};

template <typename T, typename U>
class Hashmap {
    private:
        Entry<T, U>* p; 
        size_t size_;
        size_t capacity_;
        std::vector<uint8_t> ctrl;

        Entry<T, U>* allocate_raw(size_t n) {
            return static_cast<Entry<T, U>*>(::operator new(n * sizeof(Entry<T, U>)));
        }

        void deallocate_raw(Entry<T, U>* p) {
            ::operator delete(p);
        }

        void destroy_range(Entry<T, U>* p, size_t n) noexcept {
            for(size_t i = 0; i < n; ++i) {
                if (ctrl[i] == 1) (p + i)->~Entry<T, U>();
            }
        }

        void grow() {
            const size_t new_capacity_ = capacity_ * 2;
            Entry<T, U>* new_p = allocate_raw(new_capacity_);
            std::vector<uint8_t> new_ctrl(new_capacity_, 0);

            for (size_t i = 0; i < capacity_; ++i) {
                if (ctrl[i] != 1) continue;
                
                Entry<T, U>& entry = p[i];
                const size_t start = hash(entry.key) % new_capacity_;

                for (size_t j = 0; j < new_capacity_; ++j) {
                    const size_t index = (start + j) % new_capacity_;
                    
                    if (new_ctrl[index] == 0) {
                        ::new (static_cast<void*>(new_p + index)) Entry<T, U>(std::move(entry.key), std::move(entry.value));
                        new_ctrl[index] = 1;
                        break;
                    }
                }
            }
            destroy_range(p, capacity_);
            deallocate_raw(p);
            p = new_p;
            ctrl = std::move(new_ctrl);
            capacity_ = new_capacity_;
        }

        void swap(Hashmap& other) noexcept {
            std::swap(size_, other.size_);
            std::swap(capacity_, other.capacity_);
            std::swap(ctrl, other.ctrl);
            std::swap(p, other.p);
        }

    public:
        Hashmap() : p(allocate_raw(1)), size_(0), capacity_(1), ctrl(1, 0) {};
        Hashmap(size_t capacity) : p(allocate_raw(capacity)), size_(0), capacity_(capacity), ctrl(capacity, 0) {};
        
        ~Hashmap() noexcept {
            destroy_range(p, capacity_);
            deallocate_raw(p);
        }

        // copy and move
        Hashmap(const Hashmap& other) 
            : p(allocate_raw(other.capacity_)), size_(other.size_), capacity_(other.capacity_), ctrl(other.ctrl)
        {
            for (size_t i = 0; i < capacity_; ++i) {
                if (other.ctrl[i] == 1) {
                    Entry<T, U> entry = other.p[i];
                    ::new (static_cast<void*>(p + i)) Entry<T, U>(entry);
                }
            }
        }

        Hashmap& operator=(const Hashmap& other)  {
            if (this == &other) return *this;
            Hashmap tmp(other);
            swap(tmp);
            return *this;
        }

        Hashmap(Hashmap&& other) noexcept
            : p(other.p), size_(other.size_), capacity_(other.capacity_), ctrl(std::move(other.ctrl))
        {
            other.p = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }

        Hashmap& operator=(Hashmap&& other) noexcept {
            if (this == &other) return *this;
            destroy_range(p, capacity_);
            deallocate_raw(p);

            p = other.p;
            size_ = other.size_;
            capacity_ = other.capacity_;
            ctrl = std::move(other.ctrl);

            other.p = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;

            return *this;
        }

        size_t hash(const T& key) const noexcept {
            return std::hash<T>{}(key);
        }

        void insert(const T& key, const U& value) {
            if (size_ >= (size_t)(0.7 * capacity_)) grow();

            const size_t start = hash(key) % capacity_;
            size_t first_tombstone = capacity_;
            for (size_t i = 0; i < capacity_; ++i) {
                const size_t index = (start + i) % capacity_;
                const uint8_t state = ctrl[index];
                
                if (state == 0) {
                    const size_t target = first_tombstone != capacity_ ? first_tombstone : index;
                    ::new (static_cast<void*>(p + target)) Entry<T, U>(key, value);
                    ctrl[target] = 1;
                    ++size_;
                    return;
                } else if (state == 1) {
                    Entry<T, U>& entry = p[index];
                    if (entry.key == key) {
                        entry.value = value;
                        return;
                    }
                } else {
                    if (first_tombstone == capacity_) {
                        first_tombstone = index;
                    }
                }
            }

            if (first_tombstone != capacity_) {
                ::new (static_cast<void*>(p + first_tombstone)) Entry<T, U>(key, value);
                ctrl[first_tombstone] = 1;
                ++size_;
                return;
            }
        }

        bool erase(const T& key) {
            const size_t start = hash(key) % capacity_;

            for (size_t i = 0; i < capacity_; ++i) {
                const size_t index = (start + i) % capacity_;
                const uint8_t state = ctrl[index];
                if (state == 0) {
                    return false;
                } else if (state == 1) {
                    Entry<T, U>& entry = p[index];
                    if (entry.key == key) {
                        entry.~Entry<T, U>();
                        ctrl[index] = 2;
                        --size_;
                        return true;
                    }
                }
            }
            return false;
        }

        U* find(const T& key) {
            const size_t start = hash(key) % capacity_;
            for (size_t i = 0; i < capacity_; ++i) {
                const size_t index = (start + i) % capacity_;
                const uint8_t state = ctrl[index];

                if (state == 0) {
                    return nullptr;
                } else if (state == 1) {
                    Entry<T, U>& entry = p[index];
                    if (entry.key == key) {
                        return &entry.value;
                    }
                }
            }
            return nullptr;
        }

        const U* find(const T& key) const {
            const size_t start = hash(key) % capacity_;
            for (size_t i = 0; i < capacity_; ++i) {
                const size_t index = (start + i) % capacity_;
                const uint8_t state = ctrl[index];

                if (state == 0) {
                    return nullptr;
                } else if (state == 1) {
                    const Entry<T, U>& entry = p[index];
                    if (entry.key == key) {
                        return &entry.value;
                    }
                }
            }
            return nullptr;
        }


        size_t size() const noexcept { return size_; }
        size_t capacity() const noexcept { return capacity_; }
        bool empty() const noexcept { return size_ == 0; }
};