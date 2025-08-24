// main.cpp
#include <cassert>
#include <iostream>
#include <string>
#include "RawHashmap.hpp"

template <typename K, typename V>
void dump(Hashmap<K,V>& m, const std::string& label) {
    std::cout << "== " << label
              << "  size=" << m.size()
              << " cap=" << m.capacity()
              << " empty=" << std::boolalpha << m.empty()
              << "\n";
}

int main() {
    {
        std::cout << "=== Basic insert/find/overwrite ===\n";
        Hashmap<int, std::string> m(2);   // tiny to force early grow
        dump(m, "start");

        m.insert(1, "one");
        m.insert(3, "three");
        dump(m, "after 1,3");

        if (auto* v = m.find(1)) {
            assert(*v == "one");
            *v = "ONE";
        } else { return 1; }
        assert(m.find(2) == nullptr);

        m.insert(1, "uno");
        assert(m.find(1) && *m.find(1) == std::string("uno"));

        bool erased = m.erase(2);
        assert(!erased);

        erased = m.erase(3);
        assert(erased);
        assert(m.find(3) == nullptr);
        dump(m, "after erase(3)");
    }

    {
        std::cout << "\n=== Tombstone reuse ===\n";
        Hashmap<int, std::string> m(4);
        m.insert(10, "a");
        m.insert(14, "b");
        m.insert(18, "c");
        dump(m, "after inserts");

        bool erased = m.erase(14);
        assert(erased);
        assert(m.find(14) == nullptr);
        dump(m, "after erase(14)");

        m.insert(22, "d"); // should reuse tombstone
        assert(m.find(22) && *m.find(22) == "d");
        dump(m, "after insert(22)");
    }

    {
        std::cout << "\n=== Rehash/grow preservation ===\n";
        Hashmap<int, std::string> m(2);
        for (int i = 0; i < 20; ++i) m.insert(i, "v" + std::to_string(i));
        dump(m, "after 0..19");

        for (int key : {0, 5, 9, 13, 19}) {
            auto* v = m.find(key);
            assert(v && *v == ("v" + std::to_string(key)));
        }

        const Hashmap<int, std::string>& cm = m;
        const std::string* cv = cm.find(9);
        assert(cv && *cv == "v9");
    }

    {
        std::cout << "\n=== Default constructor sanity ===\n";
        Hashmap<int, int> m;   // default cap = 1
        assert(m.empty());
        m.insert(42, 7);
        assert(m.size() == 1);
        assert(m.find(42) && *m.find(42) == 7);
        bool erased = m.erase(42);
        assert(erased);
        assert(m.empty());
        assert(m.find(42) == nullptr);
    }

    // -------- COPY SEMANTICS --------
    {
        std::cout << "\n=== Copy constructor & independence ===\n";
        Hashmap<int, std::string> a(4);
        a.insert(1, "one");
        a.insert(5, "five");
        a.insert(9, "nine");
        a.erase(5); // create a tombstone

        Hashmap<int, std::string> b = a; // copy-ctor

        // Same logical contents
        assert(b.size() == a.size());
        assert(b.find(1) && *b.find(1) == "one");
        assert(b.find(9) && *b.find(9) == "nine");
        assert(b.find(5) == nullptr); // tombstone not resurrected

        // Independence (no shared storage)
        *b.find(1) = "uno";
        assert(*b.find(1) == "uno");
        assert(*a.find(1) == "one"); // a unchanged
    }

    {
        std::cout << "\n=== Copy assignment & self-assignment ===\n";
        Hashmap<int, std::string> src(2);
        src.insert(2, "two");
        src.insert(4, "four");

        Hashmap<int, std::string> dst(8);
        dst.insert(99, "x");
        dst = src; // copy-assign

        assert(dst.size() == src.size());
        assert(dst.find(2) && *dst.find(2) == "two");
        assert(dst.find(4) && *dst.find(4) == "four");
        assert(dst.find(99) == nullptr); // overwritten

        // self-assignment safety
        dst = dst;
        assert(dst.find(2) && *dst.find(2) == "two");
        assert(dst.find(4) && *dst.find(4) == "four");
    }

    // -------- MOVE SEMANTICS --------
    {
        std::cout << "\n=== Move constructor ===\n";
        Hashmap<int, std::string> m(2);
        m.insert(7, "seven");
        m.insert(8, "eight");

        Hashmap<int, std::string> moved(std::move(m)); // after this, don't use m
        assert(moved.size() == 2);
        assert(moved.find(7) && *moved.find(7) == "seven");
        assert(moved.find(8) && *moved.find(8) == "eight");

        // mutate moved-to; just ensure it's alive & independent
        *moved.find(7) = "SEVEN";
        assert(*moved.find(7) == "SEVEN");
        // (intentionally not touching 'm' anymore; moved-from only needs to be destructible)
    }

    {
        std::cout << "\n=== Move assignment ===\n";
        Hashmap<int, std::string> a(2);
        a.insert(1, "one");
        a.insert(2, "two");

        Hashmap<int, std::string> b(2);
        b.insert(100, "hundred");

        b = std::move(a); // after this, don't use 'a'
        assert(b.size() == 2);
        assert(b.find(1) && *b.find(1) == "one");
        assert(b.find(2) && *b.find(2) == "two");
        assert(b.find(100) == nullptr); // overwritten by move-assignment target

        // self-move safety (your operator checks & returns quickly)
        b = std::move(b);
        assert(b.find(1) && *b.find(1) == "one");
    }

    std::cout << "\nAll tests passed ✅\n";
    return 0;
}
