# Task Graph

A learning project demonstrating the use of `shared_ptr` and `weak_ptr` smart pointers in C++, created while reading **Effective Modern C++**.

## Overview

This project implements a task dependency graph system where tasks can depend on other tasks. The implementation showcases proper use of smart pointers to manage object lifetimes and avoid circular reference issues.

## Key Concepts Demonstrated

- **`shared_ptr`**: Used for owning Task objects and managing their lifetimes
- **`weak_ptr`**: Used for dependencies between tasks to break potential circular reference cycles
- **Factory Pattern**: Private constructor with a static `create()` method that returns `shared_ptr<Task>`
- **Dependency Management**: Tasks track their dependencies and can only run when all dependencies have executed

## Usage Example

```cpp
// Create tasks using the factory method
shared_ptr<Task> tp1 = Task::create(add, 3, 4);
shared_ptr<Task> tp2 = Task::create(add, 1, 6);

// Create weak pointers for dependencies
weak_ptr<Task> wp1(tp1);
weak_ptr<Task> wp2(tp2);

// Establish dependencies
tp2->add_dependency(wp1);
tp1->add_dependency(wp2);

// Run tasks (will execute only when dependencies are satisfied)
tp1->run();
tp2->run();
```

## Design Decisions

- **Weak pointers for dependencies**: Using `weak_ptr` instead of `shared_ptr` for dependencies prevents circular references that could cause memory leaks
- **Private constructor**: Ensures tasks can only be created through the `create()` factory method, which guarantees proper `shared_ptr` management
- **Dependency checking**: Tasks check if their dependencies have executed before allowing execution

## Related Reading

Created while studying Chapter 4 (Smart Pointers) of **Effective Modern C++** by Scott Meyers.

