# Order Matching Engine

A limit order book matching engine implementing price-time priority matching, partial fills, and self-trade prevention — the core logic behind how stock exchanges pair buyers and sellers.

## What it does

The engine takes a stream of buy (bid) and sell (ask) orders for a single stock and automatically matches them whenever a buyer's price meets or exceeds a seller's price — exactly the way a real exchange resolves trades.

**Pipeline:**
1. A Python script generates a batch of randomized orders (`orders.json`)
2. The C++ engine reads these orders and processes them one at a time, in arrival order, matching each new order against the resting order book immediately upon arrival
3. Completed trades and the final resting order book state are written out (`trades.json`, `orderbook.json`)
4. A small JavaScript dashboard (`index.html`) reads both files and displays the live order book and trade log

## Design decisions

**Price-time priority.** Bids are stored in a max-heap (highest price wins), asks in a min-heap (lowest price wins). When two orders share the same price, the one with the earlier timestamp is served first. This is implemented using `std::priority_queue` with custom comparators, giving `O(log n)` insertion and `O(1)` access to the best available price — versus `O(n)` for a naively sorted array on every insert.

**Partial fills.** If a matching bid and ask have different quantities, the smaller quantity trades in full, and the remainder of the larger order re-enters the book as a new resting order, preserving its original timestamp (so it doesn't unfairly jump the time-priority queue).

**Resting-order pricing.** When a bid and ask cross, the trade executes at whichever order arrived first (the "resting" order) — not the incoming order's price, and not some midpoint. This reflects how real exchanges execute trades: the order that was already public and waiting sets the price; the order that just arrived accepts whatever was already available.

**Self-trade prevention.** A bid and ask from the same `userId` are never matched against each other, even if their prices would otherwise cross. When detected, the conflicting ask is temporarily set aside (not deleted) and restored once the matching engine has moved past this specific bid, so it remains eligible to trade with a different counterparty later.

**Single symbol, single-threaded.** Scoped deliberately to one instrument and no concurrency, to focus on getting the core matching logic correct. A multi-symbol version would use a separate order book (separate heap pair) per ticker, keyed by a hash map, rather than filtering by symbol inside one shared book — filtering would break the `O(log n)` guarantee that makes heaps worth using in the first place.

## Tech stack

- **C++** — matching engine core (performance-oriented, closest to how real trading systems are built)
- **Python** — randomized order generator, simulating realistic, messy market conditions
- **JavaScript** — dashboard visualizing live order book depth and trade history
- **nlohmann/json** — single-header JSON library used for reading/writing order and trade data between the Python and C++ stages

## How to run it

```bash
# 1. Generate a batch of randomized orders
python generate_orders.py

# 2. Compile and run the matching engine
g++ order.cpp -o order
./order

# 3. View the dashboard (browsers block local file fetches, so serve it)
python -m http.server 8000
# then open http://localhost:8000/index.html
```

## Sample output

```
Buy Order 4 (User 1) matched with Sell Order 6 (User 4) -> 19 shares @ Rs55.88 at t=6
Buy Order 3 (User 3) matched with Sell Order 5 (User 1) -> 17 shares @ Rs52.78 at t=5
...
```

## Possible extensions

- Multi-symbol support via a hash map of independent order books
- Market orders (in addition to limit orders)
- Order cancellation (requires handling removal from the middle of a heap, not just the top)
- Concurrency, for simulating simultaneous order arrival across multiple traders