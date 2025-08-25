from book import OrderBook
from decimal import Decimal

# ---------- helpers ----------
def assert_trade(t, price, qty, buy_id, sell_id, maker_side):
    assert t.price == Decimal(price), f"price {t.price} != {price}"
    assert t.qty == qty, f"qty {t.qty} != {qty}"
    assert t.buy_id == buy_id, f"buy_id {t.buy_id} != {buy_id}"
    assert t.sell_id == sell_id, f"sell_id {t.sell_id} != {sell_id}"
    assert t.maker_side == maker_side, f"maker_side {t.maker_side} != {maker_side}"

def best(ob):
    bb, bbq, ba, baq = ob.get_top()
    return (bb, bbq, ba, baq)

def test_scenario_a():
    """Scenario A: limit BUY crosses multiple ask levels"""
    print("Testing Scenario A: limit BUY crosses multiple ask levels...")
    ob = OrderBook()
    ob.add_order("S1", "SELL", "LIMIT", Decimal("101"), 50)   # rests
    ob.add_order("S2", "SELL", "LIMIT", Decimal("102"), 50)   # rests
    ob.add_order("B1", "BUY",  "LIMIT", Decimal("105"), 70)   # crosses 101x50 + 102x20

    assert len(ob.trades) == 2
    assert_trade(ob.trades[0], "101", 50, "B1", "S1", "SELL")
    assert_trade(ob.trades[1], "102", 20, "B1", "S2", "SELL")

    bb, bbq, ba, baq = best(ob)
    assert bb is None and bbq is None
    assert ba == Decimal("102") and baq == 30, f"best ask should be 102 x30, got {(ba,baq)}"
    assert "S2" in ob.active_orders and "S1" not in ob.active_orders
    print("✅ Scenario A passed")

def test_scenario_b():
    """Scenario B: market BUY sweeps available asks, no resting"""
    print("Testing Scenario B: market BUY sweeps available asks...")
    ob = OrderBook()
    ob.add_order("S1", "SELL", "LIMIT", Decimal("101"), 50)   # rests
    ob.add_order("S2", "SELL", "LIMIT", Decimal("102"), 50)   # rests
    ob.add_order("B1", "BUY",  "LIMIT", Decimal("105"), 70)   # crosses 101x50 + 102x20
    
    ob.add_order("B2", "BUY", "MKT", None, 25)  # sweep 25 @ 102
    assert len(ob.trades) == 3
    assert_trade(ob.trades[2], "102", 25, "B2", "S2", "SELL")

    bb, bbq, ba, baq = best(ob)
    assert ba == Decimal("102") and baq == 5
    assert "B2" not in ob.active_orders, "market orders must not rest"
    print("✅ Scenario B passed")

def test_scenario_c():
    """Scenario C: FIFO at same price (older first)"""
    print("Testing Scenario C: FIFO at same price...")
    ob2 = OrderBook()
    ob2.add_order("S3", "SELL", "LIMIT", Decimal("101"), 40)   # older at 101
    ob2.add_order("S4", "SELL", "LIMIT", Decimal("101"), 20)   # newer at 101
    ob2.add_order("B3", "BUY",  "LIMIT", Decimal("101"), 50)   # should take S3:40 then S4:10

    assert len(ob2.trades) == 2
    assert_trade(ob2.trades[0], "101", 40, "B3", "S3", "SELL")
    assert_trade(ob2.trades[1], "101", 10, "B3", "S4", "SELL")
    bb, bbq, ba, baq = best(ob2)
    assert ba == Decimal("101") and baq == 10
    assert "S4" in ob2.active_orders and "S3" not in ob2.active_orders
    print("✅ Scenario C passed")

def test_scenario_d():
    """Scenario D: limit BUY partially fills then rests remainder on bid"""
    print("Testing Scenario D: limit BUY partially fills then rests...")
    ob3 = OrderBook()
    ob3.add_order("A1", "SELL", "LIMIT", Decimal("105"), 10)   # small ask
    ob3.add_order("B4", "BUY",  "LIMIT", Decimal("105"), 25)   # fill 10 @105, rest 15 on bid@105

    assert len(ob3.trades) == 1
    assert_trade(ob3.trades[0], "105", 10, "B4", "A1", "SELL")
    bb, bbq, ba, baq = best(ob3)
    assert bb == Decimal("105") and bbq == 15, f"expected best bid 105x15, got {(bb,bbq)}"
    assert ba is None and baq is None
    assert "B4" in ob3.active_orders
    print("✅ Scenario D passed")

def test_scenario_e():
    """Scenario E: cancel a resting order and verify lazy cleanup"""
    print("Testing Scenario E: cancel order and lazy cleanup...")
    ob3 = OrderBook()
    ob3.add_order("A1", "SELL", "LIMIT", Decimal("105"), 10)   # small ask
    ob3.add_order("B4", "BUY",  "LIMIT", Decimal("105"), 25)   # fill 10 @105, rest 15 on bid@105
    
    ob3.cancel_order("B4")
    # After cancel, top-of-book call should lazily pop it
    bb, bbq, ba, baq = best(ob3)
    assert bb is None and bbq is None
    assert "B4" not in ob3.active_orders
    print("✅ Scenario E passed")

def test_scenario_f():
    """Scenario F: market SELL sweeps bids and discards any remainder"""
    print("Testing Scenario F: market SELL sweeps bids...")
    ob4 = OrderBook()
    ob4.add_order("B100", "BUY", "LIMIT", Decimal("100"), 30)   # best bid
    ob4.add_order("B99",  "BUY", "LIMIT", Decimal("99"),  15)   # next bid
    ob4.add_order("S5",   "SELL","MKT",   None,            40)   # should fill 30 @100, 10 @99

    assert len(ob4.trades) == 2
    assert_trade(ob4.trades[0], "100", 30, "B100", "S5", "BUY")  # maker BUY, taker SELL
    assert_trade(ob4.trades[1], "99",  10, "B99",  "S5", "BUY")

    bb, bbq, ba, baq = best(ob4)
    assert bb == Decimal("99") and bbq == 5, f"expected 99x5, got {(bb,bbq)}"
    assert "S5" not in ob4.active_orders
    print("✅ Scenario F passed")

def run_all_tests():
    """Run all test scenarios"""
    print("🧪 Running OrderBook tests...\n")
    
    try:
        test_scenario_a()
        test_scenario_b()
        test_scenario_c()
        test_scenario_d()
        test_scenario_e()
        test_scenario_f()
        
        print("\n🎉 All tests passed successfully! ✅")
        
    except AssertionError as e:
        print(f"\n❌ Test failed: {e}")
        raise
    except Exception as e:
        print(f"\n💥 Unexpected error: {e}")
        raise

if __name__ == "__main__":
    run_all_tests()
