#include "LockFreeStack2.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

constexpr int NUM_THREADS = 10;
std::mutex mu;


void produce(LockFreeStack<int>& st, int val, int i) {
    st.push(val);

    std::lock_guard<std::mutex> lk(mu);
    std::cout << "thread " << i << " pushed " << val << '\n';
}

void consume(LockFreeStack<int>& st, int i) {
    int val{}; 
    bool valid = st.try_pop(val);
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
    LockFreeStack<int> st{};

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(produce, std::ref(st), i * i, i);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(consume, std::ref(st), i);
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}