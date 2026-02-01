#include "ThreadSafeQueue.hpp"
#include <thread>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>

//create queue of packaged_task(void()) and produce and consume them
constexpr int CAPACITY = 10;
constexpr int PRODUCERS = 10;
constexpr int CONSUMERS = 15;

std::mutex mu;

template <typename T>
void producer(ThreadSafeQueue<T>& sq, int i, std::packaged_task<int()>&& pt) {
    bool ok = sq.push(std::move(pt));
    if (ok) {
        {
            std::lock_guard<std::mutex> lk(mu);
            std::cout << "thread " << i << " produced function\n";
        }
        
    } else {
        {
            std::lock_guard<std::mutex> lk(mu);
            std::cout << "thread " << i << " timed out\n";
        }
        
    }
}

template <typename T>
void consumer(ThreadSafeQueue<T>& sq, int i) {
    std::shared_ptr<T> sp = sq.pop();
    if (sp == nullptr) {
        {
            std::lock_guard<std::mutex> lk(mu);  
            std::cout << "thread " << i << " returned null\n"; 
        }
        
    } else {
        auto& task = *sp;
        if (task.valid()) {
            task();
            {
                std::lock_guard<std::mutex> lk(mu);
                std::cout << "thread " << i << " executed task\n";
            }
            
        }
    }
}

int square() { return 3; }

int main() {
    ThreadSafeQueue<std::packaged_task<int()>> sq(CAPACITY);
    std::vector<std::thread> threads;
    std::vector<std::future<int>> futures;

    threads.reserve(PRODUCERS + CONSUMERS);
    futures.reserve(PRODUCERS);

    for (int i = 0; i < PRODUCERS; ++i) {
        std::packaged_task<int()> pt(square);
        std::future<int> f = pt.get_future();
        threads.emplace_back(producer<std::packaged_task<int()>>, std::ref(sq), i, std::move(pt));
        futures.push_back(std::move(f));
    }

    for (int i = 0; i < CONSUMERS; ++i) {
        threads.emplace_back(consumer<std::packaged_task<int()>>, std::ref(sq), i);
    }
    
    for (auto& f : futures) {
        f.get();
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}