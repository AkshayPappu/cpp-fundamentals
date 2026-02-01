#include <atomic>
#include <thread>
#include <iostream>
#include <mutex>

std::mutex mu;

std::atomic<int> i;

std::atomic<bool> ready{false};

void producer(int n) {
    i.store(n, std::memory_order_relaxed);
    ready.store(true, std::memory_order_release);
}

void consumer(int consumer_id) {
    while (!ready.load(std::memory_order_acquire));

    while (true) {
        int new_i = i.fetch_sub(1);
        if (new_i <= 0) return;
        std::lock_guard lk(mu);
        std::cout << consumer_id << " " << new_i << '\n';
    }
}

int main() {
    std::thread t1(producer, 10);
    std::thread t2(consumer, 1);
    std::thread t3(consumer, 2);

    t1.join();
    t2.join();
    t3.join();
}