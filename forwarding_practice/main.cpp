#include "Widget.hpp"
#include <memory>
#include <iostream>
#include <utility>

// create factory function with make shared
template <typename... Ts>
std::unique_ptr<Widget> create_widget(Ts&&... ts) {
    static_assert(std::is_constructible_v<Widget, Ts&&...>, "Widget cannot be constructed with given arguments");
    return std::make_unique<Widget>(std::forward<Ts>(ts)...);
}


/* run different examples in main and see what is printed out 
    1. create widget from widget object
    2. create widget passing in valid parameters for constructor
    3. create widget passing in invalid parameters for constructor
*/
int main() {
    // calls main constructor
    int n = 5;
    std::unique_ptr<Widget> wp1 = create_widget(n);

    // call copy constructor
    std::unique_ptr<Widget> wp2 = create_widget(*wp1);

    // call move constructor
    std::unique_ptr<Widget> wp3 = create_widget(std::move(*wp1));

    // call factory function with erroneous arguments
    std::unique_ptr<Widget> wp4 = create_widget(std::move("hello"));
}