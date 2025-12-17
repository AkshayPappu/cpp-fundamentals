#pragma once
#include <fstream>
#include <string>

class StreamIterator {
    private:
        std::ifstream* _f;
        std::string _val;
    public:
        StreamIterator(std::ifstream* f);
        StreamIterator() : _f(nullptr), _val{} {};
        StreamIterator(const StreamIterator& other);
        ~StreamIterator() = default;

        StreamIterator& operator++();
        bool operator==(const StreamIterator& other) const;
        bool operator!=(const StreamIterator& other) const;
        std::string operator*() const;
};