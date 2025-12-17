#include "StreamIterator.hpp"
#include <cstddef>
#include <iostream>

using namespace std;

int main() {
    string file_name = "test.txt";
    ifstream f(file_name);
    if (!f) {
        cout << "file " << file_name << " not found";
        return EXIT_FAILURE;
    }

    StreamIterator si(&f);
    StreamIterator si2 = si;
    StreamIterator eos;
    
    while (si != eos) {
        cout << *si << "\n";
        ++si;
    }
    return 0;
}