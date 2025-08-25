#pragma once
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>
#include <algorithm>
#include <functional>

enum class Side { Buy, Sell };
enum class Type { Limit, Market };

struct Order {
    uint64_t order_id;
    long long price;
    size_t qty;
    Side side;
    uint64_t timestamp;

    Order(uint64_t order_id, long long price, size_t qty, Side side, uint64_t timestamp) 
        : order_id(order_id), price(price), qty(qty), side(side), timestamp(timestamp) {}
};

struct Trade {
    uint64_t trade_id;
    uint64_t timestamp;
    long long price;
    size_t qty;
    uint64_t buy_id;
    uint64_t sell_id;
    Side maker_side;

    Trade(uint64_t trade_id, uint64_t timestamp, long long price, size_t qty, uint64_t buy_id, uint64_t sell_id, Side maker_side)
        : trade_id(trade_id), timestamp(timestamp), price(price), qty(qty), buy_id(buy_id), sell_id(sell_id), maker_side(maker_side) {}
};

class OrderBook {
    private:
        std::map<long long, std::queue<Order>, std::greater<long long>> buys;
        std::map<long long, std::queue<Order>> sells;
        std::unordered_set<uint64_t> active_orders;
        std::vector<Trade> trades;
        uint64_t seq;
        uint64_t next_order_id;
        uint64_t next_trade_id;

        void _add_limit(Side side, long long price, size_t qty) {
            if (side == Side::Buy) {
                auto it = sells.begin();
                while (it != sells.end() && price >= it->first && qty > 0) {
                    auto& levelQ = it->second;

                    while (!levelQ.empty() && qty > 0) {
                        Order& o = levelQ.front();
                        if (active_orders.find(o.order_id) == active_orders.end()) {
                            levelQ.pop();
                            continue;
                        }

                        size_t qty_consumed = std::min(qty, o.qty);
                        qty -= qty_consumed;
                        o.qty -= qty_consumed;

                        if (o.qty == 0) {
                            levelQ.pop();
                            active_orders.erase(o.order_id);
                        }
                        trades.emplace_back(next_trade_id, seq, o.price, qty_consumed, next_order_id, o.order_id, o.side);
                        ++next_trade_id;
                    }

                    if (levelQ.empty()) {
                        it = sells.erase(it);
                    } 
                    else {
                        break;
                    }
                    
                }
                if (qty > 0) {
                    buys[price].emplace(next_order_id, price, qty, side, seq);
                    active_orders.insert(next_order_id);
                }
            }
            else {
                auto it = buys.begin();
                while (it != buys.end() && price <= it->first && qty > 0) {
                    auto& levelQ = it->second;

                    while (!levelQ.empty() && qty > 0) {
                        Order& o = levelQ.front();
                        if (active_orders.find(o.order_id) == active_orders.end()) {
                            levelQ.pop();
                            continue;
                        }

                        size_t qty_consumed = std::min(qty, o.qty);
                        qty -= qty_consumed;
                        o.qty -= qty_consumed;

                        if (o.qty == 0) {
                            levelQ.pop();
                            active_orders.erase(o.order_id);
                        }
                        trades.emplace_back(next_trade_id, seq, o.price, qty_consumed, o.order_id, next_order_id, o.side);
                        ++next_trade_id;
                    }

                    if (levelQ.empty()) {
                        it = buys.erase(it);
                    } 
                    else {
                        break;
                    }
                    
                }
                if (qty > 0) {
                    sells[price].emplace(next_order_id, price, qty, side, seq);
                    active_orders.insert(next_order_id);
                }
            }
        }

        void _add_market(Side side, size_t qty) {
            if (side == Side::Buy) {
                auto it = sells.begin();
                while (it != sells.end() && qty > 0) {
                    auto& levelQ = it->second;

                    while (!levelQ.empty() && qty > 0) {
                        Order& o = levelQ.front();
                        if (active_orders.find(o.order_id) == active_orders.end()) {
                            levelQ.pop();
                            continue;
                        }

                        size_t qty_consumed = std::min(qty, o.qty);
                        qty -= qty_consumed;
                        o.qty -= qty_consumed;

                        if (o.qty == 0) {
                            levelQ.pop();
                            active_orders.erase(o.order_id);
                        }
                        trades.emplace_back(next_trade_id, seq, o.price, qty_consumed, next_order_id, o.order_id, o.side);
                        ++next_trade_id;
                    }

                    if (levelQ.empty()) {
                        it = sells.erase(it);
                    } 
                    else {
                        break;
                    }
                }
            }
            else {
                auto it = buys.begin();
                while (it != buys.end() && qty > 0) {
                    auto& levelQ = it->second;

                    while (!levelQ.empty() && qty > 0) {
                        Order& o = levelQ.front();
                        if (active_orders.find(o.order_id) == active_orders.end()) {
                            levelQ.pop();
                            continue;
                        }

                        size_t qty_consumed = std::min(qty, o.qty);
                        qty -= qty_consumed;
                        o.qty -= qty_consumed;

                        if (o.qty == 0) {
                            levelQ.pop();
                            active_orders.erase(o.order_id);
                        }
                        trades.emplace_back(next_trade_id, seq, o.price, qty_consumed, o.order_id, next_order_id, o.side);
                        ++next_trade_id;
                    }

                    if (levelQ.empty()) {
                        it = buys.erase(it);
                    } 
                    else {
                        break;
                    }   
                }
            }
        }

    public:
        OrderBook() : seq(0), next_order_id(1), next_trade_id(1) {}

        // required methods
        uint64_t add_order(Side side, long long price, size_t qty, Type type) {
            ++seq;
            if (type == Type::Limit) {
                _add_limit(side, price, qty);
            } 
            else {
                _add_market(side, qty);
            }
            return next_order_id++;
        }

        bool cancel_order(uint64_t order_id) {
            if (active_orders.find(order_id) != active_orders.end()) {
                active_orders.erase(order_id);
                return true;
            }
            return false;
        }

        uint64_t modify_order(uint64_t order_id, Side side, long long new_price, size_t new_qty, Type type) {
            if (cancel_order(order_id)) {
                return add_order(side, new_price, new_qty, type);
            }
            return 0;
        }
};