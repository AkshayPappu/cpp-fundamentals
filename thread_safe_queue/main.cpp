#include "SafeQueue.hpp"
#include <iostream>
#include <mutex>
#include <memory>
#include <thread>


static int CAPACITY = 100;
static int PRODUCERS = 100;
static int CONSUMERS = 150;

std::mutex io_mu;

template <typename T>
void produce(SafeQueue<T>& sq, T&& obj, int i) {
    sq.push(std::forward<T>(obj));
    std::lock_guard<std::mutex> lk(io_mu);
    std::cout << "thread " << i << " pushing value\n";
}

template <typename T>
void consume(SafeQueue<T>& sq, int i) {
    std::shared_ptr<T> sp = sq.pop();
    std::lock_guard<std::mutex> lk(io_mu);
    if (sp != nullptr) {
        std::cout << "thread " << i << " popping value\n";
    } else {
        std::cout << "thread " << i << " popping null\n";
    }
}

int main() {
    SafeQueue<int> sq(CAPACITY);
    std::vector<std::thread> threads;

    for (int i = 0; i < PRODUCERS; ++i) {
        threads.emplace_back(produce<int>, std::ref(sq), i * 3,  i);
    }

    for (int i = 0; i < CONSUMERS; ++i) {
        threads.emplace_back(consume<int>, std::ref(sq), i);
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}