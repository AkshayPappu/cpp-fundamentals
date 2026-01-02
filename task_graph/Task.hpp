#pragma once
#include <vector>
#include <memory>
#include <functional>

/*
    Task Class:
        members
        - dependencies array -> weak pointers to other Task items
        - function -> unique pointer to function
        - isReady -> bool based on dependency array status
        functions:
        - destructor: just delete current task
        - print graph: prints the task dependency tree
*/

/*
    Requirements:
        - cycles logically allowed but not structurally --> dependencies should be arr of weak pointers
        - let constructor be private and give a create function that returns a shared pointer

    Todo:
        - implement isReady checking for running
        - test all functionality
        - create taskGraph class
            - give each task an id;

    TaskGraph functionality:
        - initialize with a an empty array of tasks
        - to get a certain task need to get task id
        - to add and remove dependencies, tasks need to both be created and manipulated with task ids
*/


class Task {
    private:
        std::function<void()> fn;
        std::vector<std::weak_ptr<Task>> dependencies;
        bool isReady;
        bool hasExecuted;

        Task(std::function<void()> f);
        auto find_dependency(const std::weak_ptr<Task>& other) -> std::vector<std::weak_ptr<Task>>::iterator;
    public:
        ~Task() noexcept;
        Task(const Task& other);
        Task& operator=(const Task& other);
        Task(Task&& other);
        Task& operator=(Task&& other);

        template <typename F, typename... Args>
        static std::shared_ptr<Task> create(F&& f, Args&&... args) {
            return std::shared_ptr<Task>(
                new Task(
                    std::bind(
                        std::forward<F>(f),
                        std::forward<Args>(args)...
                    )
                )
            );
        };

        bool is_ready() const noexcept;
        bool has_executed() const noexcept;
        void set_is_ready(bool val);
        void set_has_executed(bool val);
        void add_dependency(const std::weak_ptr<Task>& other); 
        void remove_dependency(const std::weak_ptr<Task>& other);
        bool has_dependency(const std::weak_ptr<Task>& other);
        void ready_to_run();
        void run(); 
        void print_graph();
};  