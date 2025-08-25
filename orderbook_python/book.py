# take in orders in (new id, side, price, quantity)
# store them and match on price-time, match at resting order price
# if remainder, store it as new order
# limit vs market: limit normal, market is fulfill immediately and discard rest

# buys stores (-price, timestamp, order_id)
# sells stores(price, timestamp, order_id)
import time
import math
import heapq
from decimal import Decimal
from dataclasses import dataclass


@dataclass
class Trade:
    trade_id: int
    timestamp: float
    price: Decimal
    qty: int
    buy_id: str
    sell_id: str
    maker_side: str

class OrderBook():
    def __init__(self):
        self.sells = []
        self.buys = []
        self.active_orders = set()
        self.trades = []
        self.start_time = time.time()

    
    def add_order(self, order_id, side, order_type, price, qty):
        # basic edge case
        if not order_id or side not in ['BUY', 'SELL'] or order_type not in ['LIMIT', 'MKT'] or not qty:
            return
        if order_type == 'LIMIT':
            self._match_limit(order_id, side, price, qty)
        if order_type == 'MKT':
            self._match_mkt(order_id, side, qty)
        return    
    
    def cancel_order(self, order_id):
        if order_id in self.active_orders:
            self.active_orders.remove(order_id)
    
    def _match_limit(self, order_id, side, price, qty):
        if not price or price < 0:
            return
        self._clean_heaps()
        if side == 'BUY':
            while self.sells and price >= self.sells[0][0] and qty > 0:
                sp, st, sq, sid = heapq.heappop(self.sells)
                
                qty_consumed = min(qty, sq)
                qty -= qty_consumed
                sq -= qty_consumed

                if sq > 0:
                    heapq.heappush(self.sells, (sp, st, sq, sid))
                else:
                    self.active_orders.remove(sid)
                trade = Trade(len(self.trades), time.time() - self.start_time, sp, qty_consumed, order_id, sid, 'SELL')
                self.trades.append(trade)
                self._clean_heaps()
            
            if qty > 0:
                heapq.heappush(self.buys, (-price, time.time() - self.start_time, qty, order_id))
                self.active_orders.add(order_id)
        else:
            while self.buys and price <= -self.buys[0][0] and qty > 0:
                bp, bt, bq, bid = heapq.heappop(self.buys)

                qty_consumed = min(qty, bq)
                qty -= qty_consumed
                bq -= qty_consumed

                if bq > 0:
                    heapq.heappush(self.buys, (bp, bt, bq, bid))
                else:
                    self.active_orders.remove(bid)
                trade = Trade(len(self.trades), time.time() - self.start_time, -bp, qty_consumed, bid, order_id, 'BUY')
                self.trades.append(trade)
                self._clean_heaps()  

            if qty > 0:
                heapq.heappush(self.sells, (price, time.time() - self.start_time, qty, order_id))
                self.active_orders.add(order_id)

    def _match_mkt(self, order_id, side, qty):
        self._clean_heaps()
        if side == 'BUY':
            while self.sells and qty > 0:
                sp, st, sq, sid = heapq.heappop(self.sells)
                
                qty_consumed = min(qty, sq)
                qty -= qty_consumed
                sq -= qty_consumed

                if sq > 0:
                    heapq.heappush(self.sells, (sp, st, sq, sid))
                else:
                    self.active_orders.remove(sid)
                trade = Trade(len(self.trades), time.time() - self.start_time, sp, qty_consumed, order_id, sid, 'SELL')
                self.trades.append(trade)
                self._clean_heaps()
        else:
            while self.buys and qty > 0:
                bp, bt, bq, bid = heapq.heappop(self.buys)

                qty_consumed = min(qty, bq)
                qty -= qty_consumed
                bq -= qty_consumed

                if bq > 0:
                    heapq.heappush(self.buys, (bp, bt, bq, bid))
                else:
                    self.active_orders.remove(bid)
                trade = Trade(len(self.trades), time.time() - self.start_time, -bp, qty_consumed, bid, order_id, 'BUY')
                self.trades.append(trade)
                self._clean_heaps()  

    def _clean_heaps(self):
        while self.buys and self.buys[0][3] not in self.active_orders:
            heapq.heappop(self.buys)
        while self.sells and self.sells[0][3] not in self.active_orders:
            heapq.heappop(self.sells)

    def get_top(self):
        best_bid = best_bid_qty = best_sell = best_sell_qty = None
        self._clean_heaps()
        if self.buys:
            best_bid, best_bid_qty = -self.buys[0][0], self.buys[0][2]
        if self.sells:
            best_sell, best_sell_qty = self.sells[0][0], self.sells[0][2]
        return [best_bid, best_bid_qty, best_sell, best_sell_qty]