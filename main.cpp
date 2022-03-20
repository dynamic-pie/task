#include <iostream>
#include <fstream>
#include <vector>

#include "lib/order_book.h"
#include "lib/parser.h"

int main() {
    std::string inputFilePath = "../data/huobi_global_depth.log";
    std::string outputFilePath = "../data/huobi_global_depth_answers.log";

    std::ifstream inputData(inputFilePath);
    std::ofstream outputData(outputFilePath);
    if (!inputData || !outputData) {
        std::cerr << "File could not be opened: " << (!inputData ? inputFilePath : outputFilePath) << std::endl;
        exit(1);
    }
    OrderBook orderBook(20, 0.01f);
    std::string line;
    getline(inputData, line);
    orderBook.InitLevels(Parser::GetBidAsk(line));
    outputData << std::setprecision(6) << std::fixed;

    while (getline(inputData, line)) {
        BidAsk bidAsk = Parser::GetBidAsk(line);
        orderBook.Update(bidAsk);
        orderBook.Dump(outputData);
    }
    inputData.close();
    outputData.close();
    return 0;
}
