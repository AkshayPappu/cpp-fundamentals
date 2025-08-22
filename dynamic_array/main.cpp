#include "array.hpp"
#include <iostream>

int main() {
    DynamicArray<int> arr(2);
    arr.push_back(5);
    arr.emplace_back(6);
    arr.emplace_back(7);
    arr.pop_back();
    for (int i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << "\n";
    }
    return 0;
}