#include "LockFreeQueue.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

constexpr int NUM_THREADS = 10;
std::mutex mu;


void produce(LockFreeQueue<int>& q, int val, int i) {
    q.try_push(val);

    std::lock_guard<std::mutex> lk(mu);
    std::cout << "thread " << i << " pushed " << val << '\n';
}

void consume(LockFreeQueue<int>& q, int i) {
    int val{}; 
    bool valid = q.try_pop(val);
    if (valid) {
        std::lock_guard<std::mutex> lk(mu);
        std::cout << "thread " << i << " popped " << val << '\n';
    } else {
        std::lock_guard<std::mutex> lk(mu);
        std::cout << "thread " << i << " failed to pop\n"; 
    }
}

int main() {
    std::vector<std::thread> threads;
    LockFreeQueue<int> q(10);

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(produce, std::ref(q), i * i, i);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(consume, std::ref(q), i);
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}