#pragma once

#include <vector>

enum OrderType {
    BID,
    ASK
};

struct PriceAmount {
    PriceAmount() = default;
    PriceAmount(double price, double amount)
    : Price_(price)
    , Amount_(amount)
    {}
    double Price_ = 0;
    double Amount_ = 0;
};

struct BidAsk {
    BidAsk() = default;
    std::vector<PriceAmount> Bids_;
    std::vector<PriceAmount> Asks_;
    int64_t EventTime_ = 0;
};
