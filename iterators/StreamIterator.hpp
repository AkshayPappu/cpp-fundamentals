#pragma once
#include <fstream>
#include <string>


class StreamIterator {
    private:
        std::ifstream* _f;
        std::string _val;
    public:
        StreamIterator(std::ifstream* f) : _f(f), _val{} {};

        bool operator()();
        
        std::string val() const;
};