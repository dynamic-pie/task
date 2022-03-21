#pragma once

#include <vector>
#include <algorithm>
#include <array>
#include <sstream>
#include <cmath>
#include "base.h"

#define MAX_CONTAINER_SIZE 256

class OrderBook {
public:
    OrderBook(int maxLevelCount, double priceEpsilon, double amountEpsilon)
    : MaxLevelCount_(maxLevelCount)
    , PriceEpsilon_(priceEpsilon)
    , AmountEpsilon_(amountEpsilon)
    , FreeAsksIndex_(maxLevelCount)
    , FreeBidsIndex_(maxLevelCount)
    {}

    void InitLevels(const BidAsk& bidAsk) noexcept {
        std::copy_n(std::make_move_iterator(bidAsk.Bids_.begin()), MaxLevelCount_, CurrentBids_.begin());
        std::copy_n(std::make_move_iterator(bidAsk.Asks_.begin()), MaxLevelCount_, CurrentAsks_.begin());
        UpdateBestAsk(CurrentAsks_, &BestAskIndex_, FreeAsksIndex_);
        UpdateBestBid(CurrentBids_, &BestBidIndex_, FreeBidsIndex_);
        CurrentEventTime_ = bidAsk.EventTime_;
    }

    PriceAmount GetBestBid() noexcept {
        return CurrentBids_[BestBidIndex_];
    }

    PriceAmount GetBestAsk() noexcept {
        return CurrentAsks_[BestAskIndex_];
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
        CurrentEventTime_ = bidAsk.EventTime_;
    }

    void AddBid(const PriceAmount& priceAmount) {
        AddOrder(priceAmount, &BestBidIndex_, &CurrentBids_, &FreeBidsIndex_, Greater, UpdateBestBid);
    }

    void AddAsk(const PriceAmount& priceAmount) {
        AddOrder(priceAmount, &BestAskIndex_, &CurrentAsks_, &FreeAsksIndex_, Less, UpdateBestAsk);
    }

    void Dump(std::ostream& stream) {
        stream << GetCurrentEventTime() << " "
               << GetBestBid().Price_ << " "
               << GetBestBid().Amount_ << " "
               << GetBestAsk().Price_ << " "
               << GetBestAsk().Amount_ << "\n";
    }

private:
    template<class Comparator, class Updater>
    void AddOrder(const PriceAmount& priceAmount,
                  size_t* bestIndex,
                  std::array<PriceAmount, MAX_CONTAINER_SIZE>* data,
                  size_t* freeIndex,
                  Comparator cmp,
                  Updater updater) {
        bool zeroAmount = fabs(priceAmount.Amount_ - 0.0) < AmountEpsilon_;
        if (EqualPrice(priceAmount, data->at(*bestIndex))) {
            if (!zeroAmount) {
                data->at(*bestIndex).Amount_ = priceAmount.Amount_;
            } else {
                std::swap(data->at(*bestIndex), data->at(--*freeIndex));
                updater(*data, bestIndex, *freeIndex);
            }
            return;
        }
        for (size_t it = 0; it < *freeIndex; ++it) {
            if (EqualPrice(data->at(it), priceAmount)) {
                if (zeroAmount) {
                    if (*freeIndex - 1 == *bestIndex) {
                        *bestIndex = it;
                    }
                    std::swap(data->at(it), data->at(--*freeIndex));
                } else {
                    data->at(it).Amount_ = priceAmount.Amount_;
                }
                return;
            }
        }
        if (cmp(priceAmount, data->at(*bestIndex))) {
            *bestIndex = *freeIndex;
        }
        data->at((*freeIndex)++) = priceAmount;
    }

    bool EqualPrice(const PriceAmount& a, const PriceAmount& b) noexcept {
        return fabs(a.Price_ - b.Price_) < PriceEpsilon_;
    }

    static bool Greater(const PriceAmount& a, const PriceAmount& b) {
        return a.Price_ > b.Price_;
    }

    static bool Less(const PriceAmount& a, const PriceAmount& b) {
        return a.Price_ < b.Price_;
    }

    static void UpdateBestAsk(const std::array<PriceAmount, MAX_CONTAINER_SIZE>& data, size_t* bestIndex, const size_t freeIndex) {
        *bestIndex = std::min_element(data.begin(), data.begin() + freeIndex, Less) - data.begin();
    }

    static void UpdateBestBid(const std::array<PriceAmount, MAX_CONTAINER_SIZE>& data, size_t* bestIndex, const size_t freeIndex) {
        *bestIndex = std::max_element(data.begin(), data.begin() + freeIndex, Less) - data.begin();
    }

    std::array<PriceAmount, MAX_CONTAINER_SIZE> CurrentBids_;
    std::array<PriceAmount, MAX_CONTAINER_SIZE> CurrentAsks_;

    size_t BestAskIndex_ = 0;
    size_t BestBidIndex_ = 0;

    int64_t CurrentEventTime_ = 0;

    int MaxLevelCount_ = 0;
    double PriceEpsilon_ = 0;
    double AmountEpsilon_ = 0;

    size_t FreeAsksIndex_ = 0;
    size_t FreeBidsIndex_ = 0;
};
