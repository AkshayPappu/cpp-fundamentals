// main.cpp
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <string>

// ---- test-only hack to inspect internals (do NOT use in prod) ----
#define private public
#include "book.hpp"   // <-- change to your actual header filename
#undef private
// -----------------------------------------------------------------

static const char* sideToStr(Side s) { return s == Side::Buy ? "Buy" : "Sell"; }

static void printTrades(const OrderBook& ob) {
    std::cout << "\n=== Trades (" << ob.trades.size() << ") ===\n";
    for (const auto& t : ob.trades) {
        std::cout << "trade_id=" << t.trade_id
                  << " ts=" << t.timestamp
                  << " px=" << t.price
                  << " qty=" << t.qty
                  << " buy_id=" << t.buy_id
                  << " sell_id=" << t.sell_id
                  << " maker_side=" << sideToStr(t.maker_side)
                  << "\n";
    }
}

static void printActive(const OrderBook& ob) {
    std::cout << "\nactive_orders: { ";
    for (auto id : ob.active_orders) std::cout << id << " ";
    std::cout << "}\n";
}

static std::vector<Order> copyQueue(std::queue<Order> q) {
    std::vector<Order> out;
    while (!q.empty()) { out.push_back(q.front()); q.pop(); }
    return out;
}

static void printBook(const OrderBook& ob) {
    std::cout << "\n=== Order Book ===\n";

    std::cout << "Asks (price ↑):\n";
    for (const auto& [px, q] : ob.sells) {
        auto lv = copyQueue(q); // copy to iterate without mutating
        std::cout << "  " << px << " :";
        for (const auto& o : lv) {
            std::cout << " [id=" << o.order_id
                      << " qty=" << o.qty
                      << " ts=" << o.timestamp
                      << "]";
        }
        std::cout << "\n";
    }

    std::cout << "Bids (price ↓):\n";
    for (const auto& [px, q] : ob.buys) {
        auto lv = copyQueue(q);
        std::cout << "  " << px << " :";
        for (const auto& o : lv) {
            std::cout << " [id=" << o.order_id
                      << " qty=" << o.qty
                      << " ts=" << o.timestamp
                      << "]";
        }
        std::cout << "\n";
    }
}

int main() {
    OrderBook ob;
    int score = 0;
    int total_tests = 0;

    // 1) Seed asks: 101 x 5 (rests)
    auto id_ask1 = ob.add_order(Side::Sell, 101, 5, Type::Limit);
    std::cout << "Added ask id=" << id_ask1 << " @101 x5\n";
    printBook(ob); printActive(ob); printTrades(ob);
    total_tests++; score += (ob.sells.size() == 1 && ob.sells[101].size() == 1) ? 1 : 0;

    // 2) Add bid 100 x 3 (won't cross; rests)
    auto id_bid1 = ob.add_order(Side::Buy, 100, 3, Type::Limit);
    std::cout << "\nAdded bid id=" << id_bid1 << " @100 x3\n";
    printBook(ob); printActive(ob); printTrades(ob);
    total_tests++; score += (ob.buys.size() == 1 && ob.buys[100].size() == 1) ? 1 : 0;

    // 3) Add bid 102 x 4 (crosses 101; executes 4 @101; 1 remains on ask)
    auto id_bid2 = ob.add_order(Side::Buy, 102, 4, Type::Limit);
    std::cout << "\nAdded bid id=" << id_bid2 << " @102 x4 (should trade 4 @101)\n";
    printBook(ob); printActive(ob); printTrades(ob);
    total_tests++; score += (ob.trades.size() == 1 && ob.sells[101].front().qty == 1) ? 1 : 0;

    // 4) Market buy 10 (consumes remaining asks; remainder discarded)
    auto id_mkt_buy = ob.add_order(Side::Buy, 0 /*ignored*/, 10, Type::Market);
    std::cout << "\nAdded market buy id=" << id_mkt_buy << " x10 (consume asks)\n";
    printBook(ob); printActive(ob); printTrades(ob);
    total_tests++; score += (ob.sells.empty() && ob.trades.size() == 2) ? 1 : 0;

    // 5) Cancel the resting bid at 100
    bool cancelled = ob.cancel_order(id_bid1);
    std::cout << "\nCancelled id=" << id_bid1 << " -> " << (cancelled ? "ok" : "fail") << "\n";
    printBook(ob); printActive(ob);
    total_tests++; score += cancelled ? 1 : 0;

    // 6) Add a new ask@100 x 10, then market sell to ensure tombstoned bid is skipped
    auto id_ask2 = ob.add_order(Side::Sell, 100, 10, Type::Limit);
    std::cout << "\nAdded ask id=" << id_ask2 << " @100 x10\n";
    printBook(ob);
    total_tests++; score += (ob.sells.size() == 1 && ob.sells[100].size() == 1) ? 1 : 0;

    auto id_mkt_sell = ob.add_order(Side::Sell, 0, 20, Type::Market);
    std::cout << "\nAdded market sell id=" << id_mkt_sell << " x20 (will cross bids; tombstones skipped)\n";
    printBook(ob); printActive(ob); printTrades(ob);
    total_tests++; score += (ob.buys.empty() && ob.trades.size() == 2) ? 1 : 0;

    // Display final scores
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "TEST RESULTS SUMMARY\n";
    std::cout << std::string(50, '=') << "\n";
    std::cout << "Total Tests: " << total_tests << "\n";
    std::cout << "Passed: " << score << "\n";
    std::cout << "Failed: " << (total_tests - score) << "\n";
    std::cout << "Score: " << score << "/" << total_tests << " (" << std::fixed << std::setprecision(1) << (score * 100.0 / total_tests) << "%)\n";
    
    if (score == total_tests) {
        std::cout << "🎉 ALL TESTS PASSED! 🎉\n";
    } else if (score >= total_tests * 0.8) {
        std::cout << "✅ Most tests passed - Good job!\n";
    } else if (score >= total_tests * 0.6) {
        std::cout << "⚠️  Some tests passed - Room for improvement\n";
    } else {
        std::cout << "❌ Many tests failed - Needs work\n";
    }
    std::cout << std::string(50, '=') << "\n";

    std::cout << "\nDone.\n";
    return 0;
}
