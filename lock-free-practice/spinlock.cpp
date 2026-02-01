#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class SpinLock {
    private:
        std::atomic<bool> b;
    public:
        SpinLock() : b(false) {};

        ~SpinLock() = default;

        void lock() {
            bool expected = false;
            bool desired = true;
            while (!b.compare_exchange_weak(expected, desired, std::memory_order_acquire, std::memory_order_relaxed)) {
                expected = false;
            }
        }

        void unlock() {
            b.store(false, std::memory_order_release);
        }

        SpinLock(const SpinLock& rhs) = delete;
        SpinLock(SpinLock&& rhs) = delete;
        SpinLock& operator=(const SpinLock& rhs) = delete;
        SpinLock& operator=(SpinLock&& rhs) = delete;
};

constexpr int NUM_THREADS = 10;
SpinLock lk{};

void print(int i) {
    lk.lock();
    std::cout << "Hello from thread " << i << '\n';
    lk.unlock();
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(print, i);
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}