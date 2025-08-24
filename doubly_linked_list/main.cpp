#include <iostream>
#include "dll.hpp"   // your DLL header

int main() {
    DLL<int> list;

    // Test push_back and push_front
    list.push_back(10);
    list.push_back(20);
    list.push_front(5);
    std::cout << "Size after pushes: " << list.size() << "\n";  // expect 3

    // Test pop_front
    auto first = list.pop_front();
    if (first) std::cout << "Popped front: " << *first << "\n"; // expect 5

    // Test pop_back
    auto last = list.pop_back();
    if (last) std::cout << "Popped back: " << *last << "\n";    // expect 20

    std::cout << "Size after pops: " << list.size() << "\n";     // expect 1

    // Test copy constructor
    DLL<int> copyList(list);
    std::cout << "Size of copy: " << copyList.size() << "\n";    // expect 1

    // Test copy assignment
    DLL<int> assignList;
    assignList = copyList;
    std::cout << "Size of assign: " << assignList.size() << "\n"; // expect 1

    // Test move constructor
    DLL<int> movedList(std::move(assignList));
    std::cout << "Size of moved list: " << movedList.size() << "\n"; // expect 1
    std::cout << "Size of old assign list: " << assignList.size() << "\n"; // expect 0

    // Test move assignment
    DLL<int> anotherList;
    anotherList.push_back(42);
    anotherList.push_back(99);

    std::cout << "Before move assign: anotherList size = " << anotherList.size() << "\n"; // expect 2
    anotherList = std::move(movedList);
    std::cout << "After move assign: anotherList size = " << anotherList.size() << "\n";  // expect 1
    std::cout << "Moved-from list size = " << movedList.size() << "\n";                   // expect 0

    return 0;
}
