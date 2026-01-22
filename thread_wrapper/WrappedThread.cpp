#include "WrappedThread.hpp"
#include <thread>
#include <stdexcept>
#include <iostream>

WrappedThread::WrappedThread(std::thread th, Policy po) : p(po), t(std::move(th)) {
    if (!t.joinable()) throw std::runtime_error("No thread");
}

WrappedThread::WrappedThread(WrappedThread&& rhs) noexcept : p(rhs.p), t(std::move(rhs.t)) {};

WrappedThread::~WrappedThread() noexcept {
    if (p == Policy::JOIN && t.joinable()) {
        t.join();
        std::cout << "thread joined\n";
    }
    else if (p == Policy::DETACH && t.joinable()) {
        t.detach();
        std::cout << "thread detached\n";
    }
}