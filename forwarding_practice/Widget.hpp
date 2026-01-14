#pragma once
#include <string>
#include <memory>

/*
    Widget class with constructor, copy, move, destructor
*/

class Widget {
    private:
        std::unique_ptr<int> n_ptr;
    public:
        Widget(int n);
        Widget(const Widget& rhs);
        Widget& operator=(const Widget& rhs);
        Widget(Widget&& rhs) noexcept;
        Widget& operator=(Widget&& rhs) noexcept;
        ~Widget();
        int get_n() const noexcept;
};