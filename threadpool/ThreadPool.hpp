#pragma once
#include <cstddef>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <vector>

class ThreadPool {
    private:
        size_t n_threads;
        std::vector<std::thread> threads;
        std::queue<std::function<void()>> q;
        std::mutex mu;
        std::condition_variable cv;
        bool stop;
    public:
        ThreadPool(size_t n_threads) : n_threads(n_threads) {
            for (size_t i = 0; i < n_threads; ++i) {
                threads.emplace_back([this] {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mu);
                            cv.wait(lock, [this] { return stop || !q.empty(); });
                            
                            if (stop && q.empty()) return false;

                            task = std::move(q.front());
                            q.pop();
                        }
                        task();
                    }
                });
            }
        }

        template <class F, class... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
            using return_type = typename std::invoke_result<F, Args...>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(mu);
                q.emplace([task] { (*task)(); });
            }

            cv.notify_one();
            return res;
        };

        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(mu);
                stop = true;
            }
            cv.notify_all();

            for (auto& t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
        };
};