#include "BasicThreadPool.hpp"
#include <iostream>
#include <mutex>

constexpr size_t NUM_THREADS = 10;
std::mutex mu;

void print_method(size_t i, int j) {
    std::lock_guard<std::mutex> lk(mu);
    std::cout << "Thread " << i << " is printing " << j << '\n';
}

int main() {
    BasicThreadPool pool(NUM_THREADS);
    std::vector<std::future<void>> futures;

    for (size_t i = 0; i < NUM_THREADS * 2; ++i) {
        auto fut = pool.submit(print_method, i, i*i);
        futures.push_back(std::move(fut));
    }

    for (size_t i = 0; i < futures.size(); ++i) {
        futures[i].get();
    }
}