#pragma once

#include <vector>
#include <algorithm>
#include <unordered_set>
#include <array>
#include <sstream>
#include "base.h"

#define MAX_CONTAINER_SIZE 256

class OrderBook {
public:
    OrderBook(int maxLevelCount, double priceEpsilon)
    : MaxLevelCount_(maxLevelCount)
    , PriceEpsilon_(priceEpsilon)
    , FreeAsksIndex_(maxLevelCount)
    , FreeBidsIndex_(maxLevelCount)
    {}

    void InitLevels(const BidAsk& bidAsk) noexcept {
        std::copy_n(std::make_move_iterator(bidAsk.Bids_.begin()), MaxLevelCount_, CurrentBids_.begin());
        std::copy_n(std::make_move_iterator(bidAsk.Asks_.begin()), MaxLevelCount_, CurrentAsks_.begin());

        BestAsk_ = bidAsk.Asks_.front();
        BestBid_ = bidAsk.Bids_.front();
        CurrentEventTime_ = bidAsk.EventTime_;
    }

    PriceAmount GetBestBid() noexcept {
        return BestBid_;
    }

    PriceAmount GetBestAsk() noexcept {
        return BestAsk_;
    }

    int64_t GetCurrentEventTime() const noexcept {
        return CurrentEventTime_;
    }

    void Update(BidAsk& bidAsk) {
        for (const auto& ask : bidAsk.Asks_) {
            AddAsk(ask);
        }
        for (const auto& bid : bidAsk.Bids_) {
            AddBid(bid);
        }
        auto cmp = [](const PriceAmount& a, const PriceAmount& b) {
            return a.Price_ < b.Price_;
        };
        if (NeedToUpdateBestAsk_) {
            BestAsk_ = *std::min_element(CurrentAsks_.begin(), CurrentAsks_.begin() + FreeAsksIndex_, cmp);
            NeedToUpdateBestAsk_ = false;
        }
        if (NeedToUpdateBestBid_) {
            BestBid_ = *std::max_element(CurrentBids_.begin(), CurrentBids_.begin() + FreeBidsIndex_, cmp);
            NeedToUpdateBestBid_ = false;
        }
        CurrentEventTime_ = bidAsk.EventTime_;
    }

    void AddBid(const PriceAmount& priceAmount) {
        AddOrder(priceAmount, &BestBid_, &NeedToUpdateBestBid_, &CurrentBids_, &FreeBidsIndex_, BidCmp);
    }

    void AddAsk(const PriceAmount& priceAmount) {
        AddOrder(priceAmount, &BestAsk_, &NeedToUpdateBestAsk_, &CurrentAsks_, &FreeAsksIndex_, AskCmp);
    }

    void Dump(std::ostream& stream) {
        stream << GetCurrentEventTime() << " "
               << GetBestBid().Price_ << " "
               << GetBestBid().Amount_ << " "
               << GetBestAsk().Price_ << " "
               << GetBestAsk().Amount_ << "\n";
    }

private:
    template<class Comparator>
    void AddOrder(const PriceAmount& priceAmount,
                  PriceAmount* bestOrder,
                  bool* needToUpdateFlag,
                  std::array<PriceAmount, MAX_CONTAINER_SIZE>* data,
                  size_t* freeIndex,
                  Comparator cmp) {
        for (size_t it = 0; it < *freeIndex; ++it) {
            if (EqualPrice(data->at(it), priceAmount)) {
                data->at(it).Amount_ = priceAmount.Amount_;
                if (data->at(it).Amount_ == 0) {
                    std::swap(data->at(it), data->at(--(*freeIndex)));
                } else {
                    if (cmp(priceAmount, *bestOrder) || EqualPrice(priceAmount, *bestOrder)) {
                        bestOrder->Amount_ = priceAmount.Amount_;
                        bestOrder->Price_ = priceAmount.Price_;
                    }
                }
                if (EqualPrice(priceAmount, *bestOrder) && priceAmount.Amount_ == 0) {
                    *needToUpdateFlag = true;
                }
                return;
            }
        }
        data->at((*freeIndex)++) = priceAmount;
        if (cmp(priceAmount, *bestOrder)) {
            *bestOrder = priceAmount;
        }
    }

    bool EqualPrice(const PriceAmount& a, const PriceAmount& b) noexcept {
        return fabs(a.Price_ - b.Price_) < PriceEpsilon_;
    }

    static bool BidCmp(const PriceAmount& a, const PriceAmount& b) {
        return a.Price_ > b.Price_;
    }

    static bool AskCmp(const PriceAmount& a, const PriceAmount& b) {
        return a.Price_ < b.Price_;
    }

    std::array<PriceAmount, MAX_CONTAINER_SIZE> CurrentBids_;
    std::array<PriceAmount, MAX_CONTAINER_SIZE> CurrentAsks_;
    PriceAmount BestBid_;
    PriceAmount BestAsk_;

    bool NeedToUpdateBestAsk_ = false;
    bool NeedToUpdateBestBid_ = false;

    int64_t CurrentEventTime_ = 0;

    int MaxLevelCount_ = 0;
    double PriceEpsilon_ = 0;

    size_t FreeAsksIndex_ = 0;
    size_t FreeBidsIndex_ = 0;
};
