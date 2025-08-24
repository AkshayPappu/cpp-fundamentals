#pragma once
#include <cstddef>
#include <vector>
#include <optional>
#include <utility>
#include <functional>

template <typename T, typename U>
struct Entry {
    T key;
    U value;
    bool tombstone;
    Entry() : key(T()), value(U()), tombstone(false) {}
    Entry(T key, U value) : key(std::move(key)), value(std::move(value)), tombstone(false) {}
    ~Entry() = default;
    Entry(const Entry&) = default;
    Entry& operator=(const Entry&) = default;
    Entry& operator=(Entry&&) noexcept = default;
    Entry(Entry&&) = default;
};

template <typename T, typename U>
class NormalHashmap {
    private:
        size_t size_;
        size_t capacity_;
        std::vector<std::optional<Entry<T, U>>> map;

        void grow() {
            const size_t new_capacity_ = capacity_ * 2;
            std::vector<std::optional<Entry<T, U>>> new_map(new_capacity_);

            for (size_t i = 0; i < capacity_; ++i) {
                if (!map[i] || (map[i] && map[i]->tombstone)) continue;

                const size_t start = hash(map[i]->key) % new_capacity_;
                for (size_t j = 0; j < new_capacity_; ++j) {
                    const size_t index = (start + j) % new_capacity_;
                    if(new_map[index]) continue;
                    if (!new_map[index]) {
                        new_map[index] = std::move(map[i]);
                        break;
                    }
                }
            }

            capacity_ = new_capacity_;
            map = std::move(new_map);
        }

        void swap(NormalHashmap& other) {
            std::swap(size_, other.size_);
            std::swap(capacity_, other.capacity_);
            std::swap(map, other.map);
        }
    public:
        NormalHashmap() : size_(0), capacity_(1), map(1) {}
        NormalHashmap(size_t capacity) : size_(0), capacity_(capacity), map(capacity) {}
        ~NormalHashmap() = default;

        //copy and move
        NormalHashmap(const NormalHashmap& other) : size_(other.size_), capacity_(other.capacity_), map(other.map) {}

        NormalHashmap& operator=(const NormalHashmap& other) {
            if (this == &other) return *this;
            NormalHashmap tmp(other);
            swap(tmp);
            return *this;
        }

        NormalHashmap(NormalHashmap&& other) noexcept
            : size_(std::exchange(other.size_, 0)), capacity_(std::exchange(other.capacity_, 0)), map(std::move(other.map)) {}
        
        NormalHashmap& operator=(NormalHashmap&& other) noexcept {
            if (this == &other) return *this;
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
            map = std::move(other.map);
            return *this;
        }

        size_t hash(const T& key) const noexcept {
            return std::hash<T>{}(key);
        }

        void insert(T key, U value) {
            if (size_ >= (size_t)(0.7 * capacity_)) grow();
            const size_t start = hash(key) % capacity_;
            size_t first_tombstone = capacity_;

            for (size_t i = 0; i < capacity_; ++i) {
                const size_t index = (start + i) % capacity_;
                
                if (map[index]) {
                    Entry<T, U>& e = *map[index];
                    if (e.tombstone) { 
                        if (first_tombstone == capacity_) first_tombstone = index;
                        continue;
                    }
                    if (e.key == key) {
                        e.value = std::move(value);
                        return;
                    }
                    continue;
                } else {
                    const size_t target = first_tombstone != capacity_ ? first_tombstone : index;
                    map[target].emplace(std::move(key), std::move(value));
                    ++size_;
                    return;
                }
                
            }

            if (first_tombstone != capacity_) {
                map[first_tombstone].emplace(std::move(key), std::move(value));
                ++size_;
            }
            return;
        }

        bool erase(const T& key) {
            const size_t start = hash(key) % capacity_;
            for (size_t i = 0; i < capacity_; ++i) {
                const size_t index = (start + i) % capacity_;
                if (!map[index]) return false;
                if (map[index]->tombstone) continue;
                
                Entry<T, U>& e = *map[index];
                if (e.key == key) {
                    e.tombstone = true;
                    --size_;
                    return true;
                }
            }
            return false;
        }

        U* find(const T& key) {
            const size_t start = hash(key) % capacity_;
            for (size_t i = 0; i < capacity_; ++i) {
                const size_t index = (start + i) % capacity_;
                if (!map[index]) return nullptr;
                if (map[index]->tombstone) continue;
                
                Entry<T, U>& e = *map[index];
                if (e.key == key) return &e.value;
            }
            return nullptr;
        }

        const U* find(const T& key) const {
            const size_t start = hash(key) % capacity_;
            for (size_t i = 0; i < capacity_; ++i) {
                const size_t index = (start + i) % capacity_;
                if (!map[index]) return nullptr;
                if (map[index]->tombstone) continue;
                
                const Entry<T, U>& e = *map[index];
                if (e.key == key) return &e.value;
            }
            return nullptr;
        }
};