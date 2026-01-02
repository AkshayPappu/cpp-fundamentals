#include "Task.hpp"
#include <memory>
#include <iostream>

using namespace std;

int add(int a, int b) { 
    cout << "running " << a << "+" << b << '\n';
    return a + b; 
};

int main() {
    shared_ptr<Task> tp1 = Task::create(add, 3, 4);
    shared_ptr<Task> tp2 = Task::create(add, 1, 6);
    weak_ptr<Task> wp1(tp1);
    weak_ptr<Task> wp2(tp2);

    tp2->add_dependency(wp1);
    tp1->add_dependency(wp2);
    tp1->run();
    tp2->run();
};