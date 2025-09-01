// main.cpp
#include <cassert>
#include <iostream>
#include <optional>

// Include your header (adjust the path/name if needed)
#include "Allocator.hpp"

// Tiny helper to unwrap std::optional<size_t>
static size_t must_alloc(std::optional<size_t> h) {
    assert(h.has_value() && "allocate() unexpectedly failed");
    return *h;
}

int main() {
    std::cout << "===== Simulated Freelist Allocator Tests =====\n";

    // ---------- Setup ----------
    const size_t HEAP = 64; // keep small so math is easy to eyeball
    Allocator alloc(HEAP);
    std::cout << "Heap size: " << HEAP << "\n";

    // ---------- Basic allocate & split ----------
    // Expect first alloc to start at 0 and split the initial free block.
    auto A = must_alloc(alloc.allocate(16));   // [0,16)
    std::cout << "A = allocate(16) -> start " << A << "\n";
    assert(A == 0);

    // Next allocation should start right after A if split worked.
    auto B = must_alloc(alloc.allocate(16));   // [16,32)
    std::cout << "B = allocate(16) -> start " << B << "\n";
    assert(B == 16);

    // And another (to create a middle hole later)
    auto C = must_alloc(alloc.allocate(16));   // [32,48)
    std::cout << "C = allocate(16) -> start " << C << "\n";
    assert(C == 32);

    // ---------- Free middle, then left; expect left-merge (coalesce) ----------
    // Free B first; then A. In an address-ordered free list, A+B should merge into [0,32)
    bool ok = alloc.deallocate(B, 16);
    std::cout << "free(B,16) -> " << (ok ? "ok" : "fail") << "\n";
    assert(ok);

    ok = alloc.deallocate(A, 16);
    std::cout << "free(A,16) -> " << (ok ? "ok" : "fail") << "\n";
    assert(ok);

    // Now allocating 24 should reuse the coalesced [0,32) region at start 0.
    auto D = must_alloc(alloc.allocate(24));   // should grab [0,24) out of [0,32)
    std::cout << "D = allocate(24) -> start " << D << "\n";
    assert(D == 0);

    // ---------- Free right, then left; expect right-merge then 3-way ----------
    ok = alloc.deallocate(C, 16);              // free [32,48)
    std::cout << "free(C,16) -> " << (ok ? "ok" : "fail") << "\n";
    assert(ok);

    ok = alloc.deallocate(D, 24);              // free [0,24)
    std::cout << "free(D,24) -> " << (ok ? "ok" : "fail") << "\n";
    assert(ok);

    // At this point, the free list should contain:
    //   [0,24) and [24,32) (if remainder from earlier) and [32,48)
    // After inserting [0,24), it should coalesce to [0,48)

    // Allocate something that needs the merged space.
    auto E = must_alloc(alloc.allocate(48));   // should use [0,48)
    std::cout << "E = allocate(48) -> start " << E << "\n";
    assert(E == 0);

    // ---------- Consume-whole-block path ----------
    // Free E; then allocate exactly 48 again — expect to consume whole node.
    ok = alloc.deallocate(E, 48);
    std::cout << "free(E,48) -> " << (ok ? "ok" : "fail") << "\n";
    assert(ok);

    auto F = must_alloc(alloc.allocate(48));
    std::cout << "F = allocate(48) -> start " << F << "\n";
    assert(F == 0);

    // ---------- Free tail and ensure order doesn’t break ----------
    ok = alloc.deallocate(F, 48);
    std::cout << "free(F,48) -> " << (ok ? "ok" : "fail") << "\n";
    assert(ok);

    // Now allocate small chunks across the front to make sure splitting continues to work
    auto G = must_alloc(alloc.allocate(8));    // [0,8)
    auto H = must_alloc(alloc.allocate(8));    // [8,16)
    auto I = must_alloc(alloc.allocate(8));    // [16,24)
    std::cout << "G/H/I allocate(8) -> " << G << "/" << H << "/" << I << "\n";
    assert(G == 0 && H == 8 && I == 16);

    // Free H and G; they should merge into [0,16)
    ok = alloc.deallocate(H, 8); assert(ok);
    ok = alloc.deallocate(G, 8); assert(ok);

    // Allocate 12 → should reuse [0,16) at start 0 via split
    auto J = must_alloc(alloc.allocate(12));
    std::cout << "J = allocate(12) -> start " << J << "\n";
    assert(J == 0);

    // Clean up last pieces
    ok = alloc.deallocate(I, 8);  assert(ok);
    ok = alloc.deallocate(J, 12); assert(ok);

    std::cout << "All tests passed ✅\n";
    return 0;
}