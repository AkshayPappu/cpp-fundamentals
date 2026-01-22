#include "WrappedThread.hpp"
#include <thread>

// function that will be passed into the thread
int add(int a, int b) {
    return a + b;
}

// function 1: create thread to run a function and give join as argument
void f1() {
    WrappedThread wt(std::thread(add, 3, 4), Policy::JOIN);
}

// function 2: create thread to run a function adn then detach
void f2() {
    WrappedThread wt(std::thread(add, 3, 4), Policy::DETACH);
}

// function 3: move wt and join
void f3() {
    WrappedThread wt1(std::thread(add, 3, 4), Policy::JOIN);
    WrappedThread wt2(std::move(wt1));
}

int main() {
    f1();
    f2();
    f3();
}