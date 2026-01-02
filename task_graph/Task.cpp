#include "Task.hpp"
#include <memory>
#include <iostream>
#include <stdexcept>
#include <functional>

Task::Task(std::function<void()> f) : fn(std::move(f)), isReady(true), hasExecuted(false) {};
Task::~Task() noexcept = default;
Task::Task(const Task& other) = delete;
Task& Task::operator=(const Task& other) = delete;
Task::Task(Task&& other) = delete;
Task& Task::operator=(Task&& other) = delete;

bool Task::is_ready() const noexcept {
    return isReady;
}

bool Task::has_executed() const noexcept {
    return hasExecuted;
}

void Task::set_is_ready(bool val) {
    isReady = val;
}

void Task::set_has_executed(bool val) {
    hasExecuted = val;
}

auto Task::find_dependency(const std::weak_ptr<Task>& other)
    -> std::vector<std::weak_ptr<Task>>::iterator {
    auto other_sp = other.lock();
    if (!other_sp) return dependencies.end();

    for (auto it = dependencies.begin(); it != dependencies.end(); ++it) {
        if (auto dep_sp = it->lock()) {
            if (dep_sp == other_sp) return it;
        }
    }
    return dependencies.end();
}

void Task::ready_to_run() {
    for (auto it = dependencies.begin(); it != dependencies.end(); ++it) {
        if (auto dep_sp = it->lock()) {
            if (!dep_sp->has_executed()) {
                set_is_ready(false);
                return;
            }
        }
    }
    set_is_ready(true);
    return;
}

void Task::add_dependency(const std::weak_ptr<Task>& other) {
    if (!has_dependency(other)) {
        dependencies.push_back(other);
    };
    ready_to_run();
};

void Task::remove_dependency(const std::weak_ptr<Task>& other) {
    auto it = Task::find_dependency(other);
    if (it != dependencies.end()) {
        dependencies.erase(it);
    };
    ready_to_run();
};

bool Task::has_dependency(const std::weak_ptr<Task>& other) {
    return Task::find_dependency(other) != dependencies.end();
}

void Task::run() { 
    ready_to_run();
    if (is_ready()) {
        fn();
        set_has_executed(true);
    } else {
        throw std::runtime_error("Dependency requirement violated.");
    }
};

void Task::print_graph() {
    std::cout << "{Task";
    for (auto& dep : dependencies) {
        if (auto p = dep.lock()) {
            p->print_graph();
        }
    }
    std::cout << '}';
};