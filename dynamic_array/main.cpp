#include "array.hpp"
#include <iostream>
#include <string>

struct Test {
    std::string name;
    Test(std::string n) : name(std::move(n)) {
        std::cout << "Construct " << name << "\n";
    }
    Test(const Test& other) : name(other.name) {
        std::cout << "Copy " << name << "\n";
    }
    Test(Test&& other) noexcept : name(std::move(other.name)) {
        std::cout << "Move " << name << "\n";
    }
    ~Test() {
        std::cout << "Destroy " << name << "\n";
    }
};


int main() {
    std::cout << "=== Basic push_back ===\n";
    DynamicArray<Test> arr;
    arr.push_back(Test("A"));  // temporary → should call move constructor inside push_back
    arr.emplace_back("B");     // constructs in place → no extra copy/move

    std::cout << "=== Copy construction ===\n";
    DynamicArray<Test> arr2(arr);   // should call your copy constructor

    std::cout << "=== Copy assignment ===\n";
    DynamicArray<Test> arr3;
    arr3 = arr2;   // should call your copy assignment

    std::cout << "=== Move construction ===\n";
    DynamicArray<Test> arr4(std::move(arr2)); // should call move constructor

    std::cout << "=== Move assignment ===\n";
    DynamicArray<Test> arr5;
    arr5 = std::move(arr3);  // should call move assignment

    std::cout << "=== End of main ===\n";
}
