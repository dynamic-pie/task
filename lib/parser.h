#pragma once

#include <optional>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>

#define JSON_KEYWORD "Get Object: "
#define JSON_KEYWORD_LEN 12
#define OPEN_FIGURE_BRACKET '{'
#define CLOSE_FIGURE_BRACKET '}'

class Parser {
public:
    static BidAsk GetBidAsk(const std::string& line) {
        BidAsk bidAsk;
        std::optional<std::string> data = GetJSONStringFromLine(line);
        if (data == std::nullopt) {
            return bidAsk;
        }
        nlohmann::json parsedJson = nlohmann::json::parse(data.value());
        bidAsk.Asks_ = std::move(ParseBidAskData(parsedJson, OrderType::ASK));
        bidAsk.Bids_ = std::move(ParseBidAskData(parsedJson, OrderType::BID));
        bidAsk.EventTime_ = parsedJson.at("event_time").get<int64_t>();
        return bidAsk;
    }

private:
    static std::optional<std::string> GetJSONStringFromLine(const std::string& line) {
        size_t foundIndex = line.find(JSON_KEYWORD);
        if (foundIndex == std::string::npos) {
            return std::nullopt;
        }
        std::stringstream jsonString;
        jsonString << OPEN_FIGURE_BRACKET;
        int brackets = 1;

        for (size_t currentIndex = foundIndex + JSON_KEYWORD_LEN + 1; brackets != 0 && currentIndex < line.length(); ++currentIndex) {
            jsonString << line[currentIndex];
            brackets += (line[currentIndex] == OPEN_FIGURE_BRACKET) - (line[currentIndex] == CLOSE_FIGURE_BRACKET);
        }

        return brackets != 0 ? std::nullopt : std::optional<std::string>(jsonString.str());
    }

    static std::vector<PriceAmount> ParseBidAskData(const nlohmann::json& parsedJson, OrderType orderType) {
        std::vector<PriceAmount> result;
        std::string key = "asks";
        if (orderType == OrderType::BID) {
            key = "bids";
        }
        auto array = parsedJson.at(key);
        for (auto& element : array) {
            result.emplace_back(element[0].get<double>(), element[1].get<double>());
        }
        return result;
    }
};