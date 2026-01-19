#include <cstddef>
#include <iostream>
#include <type_traits>
#include <tuple>
#include <utility>
#include <functional>

/*
    wrapper function to take in function and arguments and return lambda
*/
template <class F, class... Args>
auto create_lambda(F&& f, Args&&... args) {
    // get decayed type for function and decayed version of all the arguments for init capture
    using Fn = std::decay_t<F>;
    using R = std::invoke_result_t<Fn&, std::decay_t<Args>&...>;

    return [fn = std::forward<F>(f), 
        ...xs = std::decay_t<Args>(std::forward<Args>(args))]() mutable -> R { // use decay_t to get raw types to prevent dangling lvalue references
            return std::invoke(fn, xs...);
        };
}

int add(int a, int b) { return a + b; };

int main() {
    auto l = create_lambda(add, 5, 6);
    std::cout << l() << '\n';
    return 0;
}