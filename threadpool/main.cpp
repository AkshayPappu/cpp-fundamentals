#include "ThreadPool.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>

static const size_t N_THREADS = 4;
std::mutex mu;

// Example task function
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// Example void task function
void print_task(int id) {
    std::unique_lock<std::mutex> lock(mu);
    std::cout << "Task " << id << " executed by thread " 
              << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main() {
    ThreadPool pool(N_THREADS);
    
    std::cout << "ThreadPool created with " << N_THREADS << " threads" << std::endl;
    
    // Example 1: Enqueue tasks that return values
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 5; ++i) {
        futures.push_back(pool.enqueue(fibonacci, i + 10));
    }
    
    // Example 2: Enqueue void tasks
    for (int i = 0; i < 10; ++i) {
        pool.enqueue(print_task, i);
    }
    
    // Wait for all futures and print results
    std::cout << "\nFibonacci results:" << std::endl;
    for (size_t i = 0; i < futures.size(); ++i) {
        int result = futures[i].get();
        std::cout << "fibonacci(" << (i + 10) << ") = " << result << std::endl;
    }
    
    std::cout << "\nAll tasks completed!" << std::endl;
    
    return 0;
}