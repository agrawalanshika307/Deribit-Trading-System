#ifndef APICLIENT_H
#define APICLIENT_H

#include <iostream>
#include <optional>
#include <string>
#include <set>
#include <map>
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;
using namespace std::chrono;
using std::optional;
using std::nullopt;

class DeribitClient {
public:
    DeribitClient();
    ~DeribitClient();

    std::string urlEncode(const std::string& value);
    json sendRequest(const std::string& endpoint, const json& payload, const std::string& method, const std::string& token = "");
    json getAuthToken();
    json placeOrder(const std::string& token="", const std::string& instrument="", const std::string& type="", double amount=0.0, double price = 0.0);
    json modifyOrder(const string& order_id,
        const string& token="",
        const optional<double>& amount = nullopt,
        const optional<double>& contracts = nullopt,
        const optional<double>& price = nullopt,
        const optional<string>& advanced = nullopt,
        const optional<bool>& post_only = nullopt,
        const optional<bool>& reduce_only = nullopt);

    json sellOrder(const string& token="",
        const string& instrument="",
        const optional<double>& amount = nullopt,
        const optional<double>& contracts = nullopt,
        const optional<double>& price = nullopt,
        const optional<string>& type = nullopt,
        const optional<string>& trigger = nullopt,
        const optional<double>& trigger_price = nullopt);

    json cancelOrder(const std::string& orderid, const std::string& token="");
    json getOpenOrder(const std::string& token="");
    json getOrderState(const std::string& orderid, const std::string& token="");
    json getOrderBook(const std::string& symbol);

private:
    CURL* curl;

    json handleRequestError(const std::string& error_message);
};

#endif
