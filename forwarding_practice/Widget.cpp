#include "Widget.hpp"
#include <iostream>
#include <cstddef>
#include <memory>

// implement functions for widget

Widget::Widget(int n) : n_ptr(std::make_unique<int>(n)) {
    std::cout << "main constructor\n";
};

Widget::Widget(const Widget& rhs) : n_ptr(std::make_unique<int>(*rhs.n_ptr)) {
    std::cout << "copy constructor\n";
};

Widget& Widget::operator=(const Widget& rhs) {
    if (this != &rhs) {
        n_ptr = std::make_unique<int>(*rhs.n_ptr);
    }
    std::cout << "copy assignment\n";
    return *this;
}

Widget::Widget(Widget&& rhs) noexcept : n_ptr(std::move(rhs.n_ptr)) {
    std::cout << "move constructor\n";
};

Widget& Widget::operator=(Widget&& rhs) noexcept {
    if (this != &rhs) {
        n_ptr = std::move(rhs.n_ptr);
    }
    std::cout << "move assignment\n";
    return *this;
}

Widget::~Widget() = default;

int Widget::get_n() const noexcept {
    return *n_ptr;
};