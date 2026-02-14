#pragma once
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <future>
#include <functional>

class BasicThreadPool {
    private:
        size_t num_threads;
        std::atomic<bool> done;
        std::mutex mu;
        std::condition_variable cv;
        std::vector<std::thread> threads;
        std::queue<std::packaged_task<void()>> task_queue;

        void worker_thread() {
            std::packaged_task<void()> task;
            while (true) {
                std::unique_lock<std::mutex> lk(mu);
                cv.wait(lk, [this] { return done.load(std::memory_order_acquire) || !task_queue.empty(); });
                if (done.load(std::memory_order_acquire) && task_queue.empty()) {
                    break;
                }
                
                task = std::move(task_queue.front());
                task_queue.pop();
                lk.unlock();
                task();
            }
        }
    public:
        BasicThreadPool(size_t num_threads_) : num_threads(num_threads_), done(false) {
            for (size_t i = 0; i < num_threads; ++i) {
                threads.emplace_back(&BasicThreadPool::worker_thread, this);
            }
        }

        ~BasicThreadPool() {
            done.store(true, std::memory_order_release);
            cv.notify_all();
            for (auto& t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
        }
        
        template <typename F, typename... Args>
        auto submit(F&& f, Args&&... args) 
            -> std::future<std::invoke_result_t<F, Args...>>
        {
            using R = typename std::invoke_result_t<F, Args...>;
            std::packaged_task<R()> task(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            std::future<R> fut(task.get_future());
            
            std::unique_lock<std::mutex> lk(mu);
            task_queue.push(std::packaged_task<void()>([t = std::move(task)]() mutable { t(); }));
            lk.unlock();
            cv.notify_one();
            return fut;
        }
};