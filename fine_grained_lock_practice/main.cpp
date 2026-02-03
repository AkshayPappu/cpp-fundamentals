#include "NodeQueue.hpp"
#include <iostream>
#include <thread>
#include <vector>

std::mutex mu;
constexpr int NUM_THREADS = 10;

void produce(NodeQueue<int>& q, int val, int i) {
    q.push(val);
    {
        std::lock_guard<std::mutex> lk(mu);
        std:: cout << "thread " << i << " pushed " << val << '\n';
    }
}

void consume(NodeQueue<int>& q, int i) {
    std::shared_ptr p = q.wait_and_pop();
    if (p != nullptr)
    {
        std::lock_guard<std::mutex> lk(mu);
        std:: cout << "thread " << i << " popped " << *p << '\n';
    }
}

int main() {
    NodeQueue<int> q{};
    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(consume, std::ref(q), i + NUM_THREADS);
    }

    for (int i = 0; i < NUM_THREADS - 1; ++i) {
        threads.emplace_back(produce, std::ref(q), i, i);
    }
    
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}