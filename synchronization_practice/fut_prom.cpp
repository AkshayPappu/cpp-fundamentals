#include <thread>
#include <future>
#include <vector>
#include <iostream>
#include <mutex>

/*
    create a vector of promises and a vector of futures and pass the promises to threads and then get the futures and see the results
*/

static const int NUM_THREADS = 10;

// square function
int square(int i) { return i * i; };

int main() {
    std::mutex mu;
    std::vector<std::thread> threads;
    std::vector<std::future<int>> fv;

    for (int i = 0; i < NUM_THREADS; ++i) {
        std::promise<int> p;
        fv.push_back(p.get_future());

        threads.emplace_back([&, i, p = std::move(p)]() mutable {
            int val = square(i);
            p.set_value(val);
            {
                std::lock_guard<std::mutex> lk(mu);
                std::cout << "thread " << i << " has set promise value " << val << '\n';
            }
            
        });
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        int val = fv[i].get();
        {
            std::lock_guard<std::mutex> lk(mu);
            std::cout << "future " << i << " has received value " << val << '\n'; 
        }
        
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}