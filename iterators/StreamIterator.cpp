#include "StreamIterator.hpp"
#include <cstddef>
#include <cassert>

using namespace std;

StreamIterator::StreamIterator(std::ifstream* f) {
    if (*f >> _val) {
        _f = f;
    } else {
        _f = nullptr;
        _val = {};
    }
}

StreamIterator::StreamIterator(const StreamIterator& other) {
    _f = other._f;
    _val = other._val;
}

StreamIterator& StreamIterator::operator++() {
    if (!_f) {
        return *this;
    }
    if ((*_f >> _val)) {
        return *this;
    }
    _f = nullptr;
    _val = {};
    return *this;
}

bool StreamIterator::operator==(const StreamIterator& other) const{
    return this->_f == nullptr && other._f == nullptr;
}

bool StreamIterator::operator!=(const StreamIterator& other) const {
    return !(this->_f == nullptr && other._f == nullptr);
};

string StreamIterator::operator*() const {
    assert(_f != nullptr && "Dereferencing end StreamIterator");
    return _val;
}