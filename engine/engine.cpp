#include "engine.hpp"
#include <algorithm>
#include <cstring>

std::vector<Trade> MatchingEngine::submit_market_order(int seq_num, Side side,
                                                       const char* symbol,
                                                       uint32_t qty){
    std::vector<Trade> fills;

    Order incoming{seq_num, OrdType::Market, side, {}, qty, 0};
    strncpy(incoming.symbol, symbol, 8);

    auto it = books_.find(std::string(symbol, 8));
    if(it != books_.end()){
        match(incoming, it->second, fills);
    }

    return fills;
}

std::vector<Trade> MatchingEngine::submit_limit_order(int seq_num, Side side,
                                                       const char* symbol,
                                                       uint32_t qty, int64_t price) {
    std::vector<Trade> fills;

    Order incoming{seq_num, OrdType::Limit, side, {}, qty, price};
    strncpy(incoming.symbol, symbol, 8);

    SymbolBook& book = books_[std::string(symbol, 8)];
    match(incoming, book, fills);

    if (incoming.qty > 0) {
        if (side == Side::Buy)
            book.buy_side[price][seq_num] = incoming;
        else
            book.sell_side[price][seq_num] = incoming;
    }
    return fills;
}

bool MatchingEngine::cancel_order(int seq_num, int64_t price,
                                   Side side, const char* symbol) {
    auto book_it = books_.find(std::string(symbol, 8));
    if (book_it == books_.end()) return false;
    SymbolBook& book = book_it->second;

    auto& side_map = (side == Side::Buy) ? book.buy_side : book.sell_side;
    auto price_it = side_map.find(price);
    if (price_it == side_map.end()) return false;

    auto order_it = price_it->second.find(seq_num);
    if (order_it == price_it->second.end()) return false;

    price_it->second.erase(order_it);
    if (price_it->second.empty()) side_map.erase(price_it);

    return true;
}

std::vector<Order> MatchingEngine::get_orderbook_snapshot() const {
    std::vector<Order> snapshot;
    for (const auto& [symbol, book] : books_) {
        for (const auto& [price, level] : book.buy_side)
            for (const auto& [seq, order] : level)
                snapshot.push_back(order);

        for (const auto& [price, level] : book.sell_side)
            for (const auto& [seq, order] : level)
                snapshot.push_back(order);
    }
    return snapshot;
}

// Priority rules:
//   1. Best price first (highest bid / lowest ask).
//   2. For equal prices: lowest seq_num first (time priority).
//
// Execution price is always the resting order's price.
void MatchingEngine::match(Order& incoming, SymbolBook& book,
                            std::vector<Trade>& fills) {
    bool is_buy    = (incoming.side == Side::Buy);
    bool is_market = (incoming.ord_type == OrdType::Market);

    while(incoming.qty > 0){
        if(is_buy){
            if (book.sell_side.empty()) break;

            auto level_it = book.sell_side.begin();
            int64_t best_ask = level_it->first;

            if(!is_market && incoming.price < best_ask) break;

            auto& level    = level_it->second;
            auto  order_it = level.begin();         
            Order& resting = order_it->second;

            uint32_t fill = std::min(incoming.qty, resting.qty);

            Trade trade{};
            trade.trade_id       = next_trade_id_++;
            trade.seq_num_buy    = incoming.seq_num;
            trade.seq_num_sell   = resting.seq_num;
            trade.qty_filled     = fill;
            trade.execution_price = resting.price;
            memcpy(trade.symbol, incoming.symbol, 8);
            fills.push_back(trade);

            incoming.qty -= fill;
            resting.qty  -= fill;

            if(resting.qty == 0){
                level.erase(order_it);
                if(level.empty()) book.sell_side.erase(level_it);
            }

        } 
        else{
            if (book.buy_side.empty()) break;

            auto level_it = std::prev(book.buy_side.end());
            const int64_t best_bid = level_it->first;

            if (!is_market && incoming.price > best_bid) break;

            auto& level    = level_it->second;
            auto  order_it = level.begin();         
            Order& resting = order_it->second;

            uint32_t fill = std::min(incoming.qty, resting.qty);

            Trade trade{};
            trade.trade_id        = next_trade_id_++;
            trade.seq_num_buy     = resting.seq_num;
            trade.seq_num_sell    = incoming.seq_num;
            trade.qty_filled      = fill;
            trade.execution_price = resting.price;
            memcpy(trade.symbol, incoming.symbol, 8);
            fills.push_back(trade);

            incoming.qty -= fill;
            resting.qty  -= fill;

            if(resting.qty == 0){
                level.erase(order_it);
                if (level.empty()) book.buy_side.erase(level_it);
            }
        }
    }
}