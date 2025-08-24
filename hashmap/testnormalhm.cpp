// main.cpp
#include <cassert>
#include <iostream>
#include <string>

// Change this to your actual header filename or keep it in the same dir.
#include "NormalHashmap.hpp"

using std::cout;
using std::endl;

template <typename T>
void expect_true(T cond, const char* msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << "\n";
        std::exit(1);
    }
}

template <typename T, typename U>
void expect_eq(const T& a, const U& b, const char* msg) {
    if (!(a == b)) {
        std::cerr << "[FAIL] " << msg << " (got " << a << " vs " << b << ")\n";
        std::exit(1);
    }
}

int main() {
    cout << "=== Basic insert/find ===\n";
    NormalHashmap<std::string, int> hm(4);
    hm.insert("a", 1);
    hm.insert("b", 2);
    hm.insert("c", 3);

    auto* pa = hm.find("a");
    auto* pb = hm.find("b");
    auto* pc = hm.find("c");
    expect_true(pa && pb && pc, "find should return non-null for present keys");
    expect_eq(*pa, 1, "value for 'a' should be 1");
    expect_eq(*pb, 2, "value for 'b' should be 2");
    expect_eq(*pc, 3, "value for 'c' should be 3");

    cout << "=== Overwrite existing key ===\n";
    hm.insert("a", 10);
    pa = hm.find("a");
    expect_true(pa, "'a' should still exist after overwrite");
    expect_eq(*pa, 10, "value for 'a' should be overwritten to 10");

    cout << "=== Erase + verify missing ===\n";
    bool erased = hm.erase("b");
    expect_true(erased, "erase('b') should succeed");
    expect_true(hm.find("b") == nullptr, "'b' should be gone after erase");
    expect_true(hm.erase("missing") == false, "erase of missing key returns false");

    cout << "=== Tombstone reuse (insert after erase) ===\n";
    hm.insert("d", 4); // likely to reuse a tombstone or empty slot
    auto* pd = hm.find("d");
    expect_true(pd, "'d' should be found after insert");
    expect_eq(*pd, 4, "value for 'd' should be 4");

    cout << "=== Growth / rehash check ===\n";
    // Insert many to trigger rehash (load factor threshold ~0.7)
    for (int i = 0; i < 20; ++i) {
        hm.insert("k" + std::to_string(i), i);
    }
    // Verify a few random ones
    for (int i : {0, 1, 5, 10, 19}) {
        auto* pv = hm.find("k" + std::to_string(i));
        expect_true(pv, "key from batch should exist after rehash");
        expect_eq(*pv, i, "value should match inserted value after rehash");
    }

    cout << "=== const find() ===\n";
    const auto& cref = hm;
    const int* pconst = cref.find("k5");
    expect_true(pconst && *pconst == 5, "const find should see 'k5' = 5");
    expect_true(cref.find("does_not_exist") == nullptr, "const find missing -> nullptr");

    cout << "=== Copy constructor deep copy ===\n";
    NormalHashmap<std::string, int> copy = hm; // copy-ctor
    auto* p_in_copy = copy.find("k10");
    auto* p_in_orig = hm.find("k10");
    expect_true(p_in_copy && p_in_orig, "both copies should contain 'k10'");
    expect_eq(*p_in_copy, *p_in_orig, "'k10' values should match after copy");

    // Mutate copy; original should remain unchanged
    copy.insert("k10", 999);
    expect_true(copy.find("k10") && *copy.find("k10") == 999, "copy should reflect new value");
    expect_true(hm.find("k10") && *hm.find("k10") == 10, "original should remain 10");

    cout << "=== Move constructor ===\n";
    NormalHashmap<std::string, int> moved(std::move(copy));
    // Don't touch 'copy' after move (it may be in an empty/moved-from state).
    expect_true(moved.find("k10") && *moved.find("k10") == 999, "moved map keeps elements");

    cout << "=== Copy assignment ===\n";
    NormalHashmap<std::string, int> hm2;
    hm2 = hm; // copy-assign
    expect_true(hm2.find("a") && *hm2.find("a") == 10, "copy-assigned map contains 'a'=10");
    expect_true(hm2.find("k19") && *hm2.find("k19") == 19, "copy-assigned map contains 'k19'=19");

    cout << "=== Move assignment ===\n";
    NormalHashmap<std::string, int> hm3;
    hm3 = std::move(hm2);
    expect_true(hm3.find("k0") && *hm3.find("k0") == 0, "move-assigned map contains 'k0'=0");
    expect_true(hm3.find("a") && *hm3.find("a") == 10, "move-assigned map contains 'a'=10");

    cout << "\nAll tests passed ✅\n";
    return 0;
}
