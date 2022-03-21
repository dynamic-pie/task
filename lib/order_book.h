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
        UpdateBestAsk();
        UpdateBestBid();
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
        UpdateBestAsk();
        UpdateBestBid();
        CurrentEventTime_ = bidAsk.EventTime_;
    }

    void AddBid(const PriceAmount& priceAmount) {
        AddOrder(priceAmount, &BestBidIndex_, &NeedToUpdateBestBid_, &CurrentBids_, &FreeBidsIndex_, Greater);
    }

    void AddAsk(const PriceAmount& priceAmount) {
        AddOrder(priceAmount, &BestAskIndex_, &NeedToUpdateBestAsk_, &CurrentAsks_, &FreeAsksIndex_, Less);
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
                  size_t* bestIndex,
                  bool* needToUpdateFlag,
                  std::array<PriceAmount, MAX_CONTAINER_SIZE>* data,
                  size_t* freeIndex,
                  Comparator cmp) {
        bool zeroAmount = fabs(priceAmount.Amount_ - 0.0) < AmountEpsilon_;
        if (EqualPrice(priceAmount, data->at(*bestIndex)) && !zeroAmount) {
            data->at(*bestIndex).Amount_ = priceAmount.Amount_;
            return;
        }
        for (size_t it = 0; it < *freeIndex; ++it) {
            if (EqualPrice(data->at(it), priceAmount)) {
                if (zeroAmount) {
                    if (EqualPrice(priceAmount, data->at(*bestIndex))) {
                        *needToUpdateFlag = true;
                    }
                    if (*freeIndex - 1 == *bestIndex) {
                        *bestIndex = it;
                    }
                    std::swap(data->at(it), data->at(--*freeIndex));
                } else {
                    data->at(it).Amount_ = priceAmount.Amount_;
                    if (EqualPrice(priceAmount, data->at(*bestIndex))) {
                        *bestIndex = it;
                    }
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

    void UpdateBestAsk() {
        if (NeedToUpdateBestAsk_) {
            BestAskIndex_ = std::min_element(CurrentAsks_.begin(), CurrentAsks_.begin() + FreeAsksIndex_, Less) - CurrentAsks_.begin();
            NeedToUpdateBestAsk_ = false;
        }
    }

    void UpdateBestBid() {
        if (NeedToUpdateBestBid_) {
            BestBidIndex_ = std::max_element(CurrentBids_.begin(), CurrentBids_.begin() + FreeBidsIndex_, Less) - CurrentBids_.begin();
            NeedToUpdateBestBid_ = false;
        }
    }

    std::array<PriceAmount, MAX_CONTAINER_SIZE> CurrentBids_;
    std::array<PriceAmount, MAX_CONTAINER_SIZE> CurrentAsks_;

    bool NeedToUpdateBestAsk_ = true;
    bool NeedToUpdateBestBid_ = true;

    size_t BestAskIndex_ = 0;
    size_t BestBidIndex_ = 0;

    int64_t CurrentEventTime_ = 0;

    int MaxLevelCount_ = 0;
    double PriceEpsilon_ = 0;
    double AmountEpsilon_ = 0;

    size_t FreeAsksIndex_ = 0;
    size_t FreeBidsIndex_ = 0;
};
