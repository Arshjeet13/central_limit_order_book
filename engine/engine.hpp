#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

enum class Side   : char { buy = '1', sell = '2' };
enum class OrdType: char { market = '1', limit = '2' };

struct Trade {
    int      trade_id;
    int      seq_num_buy;
    int      seq_num_sell;
    char     symbol[8];
    uint32_t qty_filled;
    int64_t  execution_price;
};

struct Order {
    int      seq_num;
    OrdType  ord_type;
    Side     side;
    char     symbol[8];
    uint32_t qty;
    int64_t  price;
};

struct SymbolBook {
    // for every price, it stores a map of seq_num (representing time) and the related order
    std::map<int64_t, std::map<int, Order>> buy_side;
    std::map<int64_t, std::map<int, Order>> sell_side;
};

class MatchingEngine {
public:
    std::vector<Trade> submit_limit_order(int seq_num, Side side,
                                          const char* symbol,
                                          uint32_t qty, int64_t price);

    std::vector<Trade> submit_market_order(int seq_num, Side side,
                                           const char* symbol,
                                           uint32_t qty);

    bool cancel_order(int seq_num, int64_t price,
                      Side side, const char* symbol);

    std::vector<Order> get_orderbook_snapshot() const;

private:
    std::unordered_map<std::string, SymbolBook> books_;
    int next_trade_id_ = 1;

    void match(Order& incoming, SymbolBook& book,
               std::vector<Trade>& fills);
};