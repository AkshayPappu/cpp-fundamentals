#include <atomic>
#include <queue>
#include <thread>
#include <iostream>

bool x = false;
std::atomic<bool> y = false;
std::queue<int> q;

void produce() {
    x = true;
    std::atomic_thread_fence(std::memory_order_release);
    y.store(true, std::memory_order_relaxed);
}

void consume() {
    while (!y.load(std::memory_order_relaxed));
    std::atomic_thread_fence(std::memory_order_acquire);
    if (x) {
        q.push(x);
    }
}

int main() {
    std::thread t1(produce);
    std::thread t2(consume);

    t1.join();
    t2.join();
    int q_size = static_cast<int>(q.size());
    for (int i = 0; i < q_size; ++i) {
        std::cout << q.front() << '\n';
        q.pop();
    }
}