#include "StreamIterator.hpp"
#include <cstddef>


using namespace std;

bool StreamIterator::operator()() {
    return static_cast<bool>(*_f >> _val);
}

string StreamIterator::val() const {
    return _val;
}