#pragma once
#include <thread>

enum class Policy { JOIN, DETACH };

class WrappedThread {
    private:
        Policy p;
        std::thread t;
    public:
        WrappedThread(std::thread th, Policy po);
        WrappedThread(WrappedThread&& rhs) noexcept;

        // delete copy operations and move assignment
        WrappedThread(const WrappedThread& rhs) = delete;
        WrappedThread& operator=(const WrappedThread& rhs) = delete;
        WrappedThread& operator=(WrappedThread&& rhs) = delete;

        ~WrappedThread() noexcept;
};