#include <benchmark/benchmark.h>
#include <vector>
#include <iomanip>
#include <fstream>
#include <iostream>
#include "../lib/order_book.h"
#include "../lib/parser.h"

static void BM_FullOrderBookUpdate_80K(benchmark::State& state) {
    std::string inputFilePath = "../../data/huobi_global_depth.log";
    std::ifstream inputData(inputFilePath);
    if (!inputData) {
        std::cerr << "File could not be opened: " << inputFilePath << std::endl;
        exit(1);
    }

    std::string line;
    getline(inputData, line);
    auto startState = Parser::GetBidAsk(line);

    std::vector<BidAsk> bidAsks;

    while (getline(inputData, line)) {
        bidAsks.emplace_back(Parser::GetBidAsk(line));
    }
    inputData.close();

    for (auto _: state) {
        OrderBook orderBook(20, 0.01f);
        orderBook.InitLevels(startState);
        for (auto&& bidAsk: bidAsks) {
            orderBook.Update(bidAsk);
        }
    }
}
BENCHMARK(BM_FullOrderBookUpdate_80K);


static void BM_SingleOrderBookUpdate(benchmark::State& state) {
    std::string inputFilePath = "../../data/huobi_global_depth.log";
    std::ifstream inputData(inputFilePath);
    if (!inputData) {
        std::cerr << "File could not be opened: " << inputFilePath << std::endl;
        exit(1);
    }

    std::string line;
    getline(inputData, line);
    auto startState = Parser::GetBidAsk(line);

    std::vector<BidAsk> bidAsks;

    while (getline(inputData, line)) {
        bidAsks.emplace_back(Parser::GetBidAsk(line));
    }
    inputData.close();

    OrderBook orderBook(20, 0.01f);
    orderBook.InitLevels(startState);
    size_t index = 0;
    for (auto _: state) {
        orderBook.Update(bidAsks[index++]);
        if (index >= bidAsks.size()) {
            index = 0;
            orderBook = OrderBook(20, 0.01f);
            orderBook.InitLevels(startState);
        }
    }
}
BENCHMARK(BM_SingleOrderBookUpdate);

BENCHMARK_MAIN();