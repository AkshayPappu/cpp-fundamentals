#pragma once
#include "SafeQueue.hpp"
#include <vector>
#include <thread>
#include <functional>
#include <future>
#include <atomic>
#include <stdexcept>

class ThreadPool {
    private:
        size_t num_threads;
        std::vector<std::thread> threads;
        SafeQueue<std::packaged_task<void()>> task_queue;

        void worker_thread() {
            std::packaged_task<void()> task;
            while (task_queue.wait_and_pop(task)) {
                task();
            }
        }

    public:
        ThreadPool(size_t num_threads_) : num_threads(num_threads_), task_queue(num_threads_) {
            for (size_t i = 0; i <num_threads; ++i) {
                threads.emplace_back(&ThreadPool::worker_thread, this);
            }
        }

        ~ThreadPool() {
            task_queue.close();
            for (auto& t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
        }

        std::future<void> submit(std::function<void()> f) {
            std::packaged_task<void()> task(std::move(f));
            std::future<void> fut = task.get_future();
            if (!task_queue.wait_and_push(std::move(task))) {
                throw std::runtime_error("ThreadPool is shutting down");
            }
            return fut;
        }
};