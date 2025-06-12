#include "APIClient.h"       // Include the header file for the API client
#include <iostream>          // For input/output operations
#include <optional>          // For using optional types
#include <string>            // For string operations
#include <set>               // For set data structure
#include <map>               // For map data structure
#include <curl/curl.h>       // For handling HTTP requests via libcurl
#include <thread>            // For multithreading
#include <chrono>            // For time-related functions
#include <mutex>             // For thread-safe operations
#include <nlohmann/json.hpp> // For JSON handling using the nlohmann library

using json = nlohmann::json; // Alias for JSON type from nlohmann library
using namespace std;         // Using the standard namespace
using namespace std::chrono; // For time utilities
using std::nullptr_t;        // Null optional value
using std::optional;         // Optional type for handling optional values

// Constants for API URL and credentials
const std::string API_URL = "https://test.deribit.com";
const std::string CLIENT_ID = "YOUR CLIENT ID";                                        // Replace with your client ID
const std::string CLIENT_SECRET = "YOUR CLIENT SECRET"; // Replace with your client secret

// Helper function for handling libcurl responses
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb); // Append response data to a string
    return size * nmemb;                                            // Return the size of the processed data
}

// Constructor for the DeribitClient class
DeribitClient::DeribitClient()
{
    curl_global_init(CURL_GLOBAL_ALL); // Initialize libcurl globally
    curl = curl_easy_init();           // Initialize a CURL easy session
}

// Destructor for the DeribitClient class
DeribitClient::~DeribitClient()
{
    if (curl)
        curl_easy_cleanup(curl); // Clean up the CURL session
    curl_global_cleanup();       // Clean up global CURL resources
}

// URL encoding utility
std::string DeribitClient::urlEncode(const std::string &value)
{
    CURL *curl = curl_easy_init();
    char *encoded = curl_easy_escape(curl, value.c_str(), value.length()); // Encode URL
    std::string encoded_str(encoded);                                      // Convert to string
    curl_free(encoded);                                                    // Free the encoded result
    curl_easy_cleanup(curl);                                               // Clean up CURL session
    return encoded_str;
}

// Function to send HTTP requests
json DeribitClient::sendRequest(const std::string &endpoint, const json &payload, const std::string &method, const std::string &token)
{
    std::string response;                 // String to store the HTTP response
    std::string url = API_URL + endpoint; // Construct the full URL

    // Append query parameters for GET requests
    if (method == "GET" && !payload.empty())
    {
        url += "?";
        for (auto it = payload.begin(); it != payload.end(); ++it)
        {
            if (it.value().is_string())
            {
                url += it.key() + "=" + it.value().get<std::string>() + "&"; // Add key-value pair
            }
            else
            {
                url += it.key() + "=" + it.value().dump() + "&"; // Add non-string values
            }
        }
        url.pop_back(); // Remove trailing '&'
    }

    // Setup HTTP headers
    struct curl_slist *headers = nullptr;
    if (!token.empty())
    {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str()); // Add authorization header
    }
    headers = curl_slist_append(headers, "Content-Type: application/json"); // Add content-type header

    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());    // Set URL
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); // Set headers
    if (method == "POST")
    {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);                        // Set POST method
        std::string payload_str = payload.dump();                        // Serialize JSON payload
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str()); // Add POST data
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // Set write callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);         // Pass the response string

    CURLcode res = curl_easy_perform(curl); // Perform the request
    curl_slist_free_all(headers);           // Free allocated headers

    if (res != CURLE_OK)
    { // Check for CURL errors
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        return {};
    }

    return json::parse(response); // Parse and return the JSON response
}

// Authentication to get a token function.
json DeribitClient::getAuthToken()
{
    json payload = {
        {"grant_type", "client_credentials"},
        {"client_id", CLIENT_ID},
        {"client_secret", CLIENT_SECRET}};

    json response = sendRequest("/api/v2/public/auth", payload, "GET");

    if (response.contains("result") && response["result"].contains("access_token"))
    {
        return response["result"]["access_token"];
    }

    std::cerr << "Failed to authenticate: " << response.dump(4) << std::endl;
    return "";
}

// Place an order
json DeribitClient::placeOrder(const std::string &token, const std::string &instrument, const std::string &type, double amount, double price)
{
    json payload = {
        {"instrument_name", instrument},
        {"type", type},
        {"amount", amount}};

    if (type == "limit")
    {
        payload["price"] = price;
    }

    cout << "Placing order in progress..." << endl;
    return sendRequest("/api/v2/private/buy", payload, "GET", token);
}

// Function to modify an existing order on the Deribit platform
json DeribitClient::modifyOrder(const std::string &order_id,
                                const std::string &token,
                                const optional<double> &amount,
                                const optional<double> &contracts,
                                const optional<double> &price,
                                const optional<std::string> &advanced,
                                const optional<bool> &post_only,
                                const optional<bool> &reduce_only)
{
    if (amount && contracts && *amount != *contracts)
    {
        cerr << "Error: 'amount' and 'contracts' must match if both are provided." << endl;
        return {};
    }

    if (!amount && !contracts)
    {
        cerr << "Error: Either 'amount' or 'contracts' must be provided." << endl;
        return {};
    }

    json payload;
    payload["order_id"] = order_id;
    if (amount)
        payload["amount"] = *amount;
    if (contracts)
        payload["contracts"] = *contracts;
    if (price)
        payload["price"] = *price;
    if (advanced)
        payload["advanced"] = *advanced;
    if (post_only)
        payload["post_only"] = *post_only;
    if (reduce_only)
        payload["reduce_only"] = *reduce_only;

    cout << "Modifying order in progress..." << endl;

    return sendRequest("/api/v2/private/edit", payload, "GET", token);
}

// Function to place a sell order on the Deribit platform
json DeribitClient::sellOrder(const string &token,
                              const string &instrument,
                              const optional<double> &amount,
                              const optional<double> &contracts,
                              const optional<double> &price,
                              const optional<string> &type,
                              const optional<string> &trigger,
                              const optional<double> &trigger_price)
{
    json payload;
    payload["instrument_name"] = instrument;
    if (amount)
        payload["amount"] = *amount;
    if (contracts)
        payload["contracts"] = *contracts;
    if (price)
        payload["price"] = *price;
    if (type)
        payload["type"] = *type;
    if (trigger)
        payload["trigger"] = *trigger;
    if (trigger_price)
        payload["trigger_price"] = *trigger_price;

    cout << "Selling in progress..." << endl;

    return sendRequest("/api/v2/private/sell", payload, "GET", token);
}

// Function to cancel a specific order by its ID
json DeribitClient::cancelOrder(const std::string &order_id, const std::string &token)
{
    cout << "Cancelling order in progress..." << endl;
    json payload = {
        {"order_id", order_id}};

    return sendRequest("/api/v2/private/cancel", payload, "GET", token);
}

// Function to get all open orders for the authenticated user
json DeribitClient::getOpenOrder(const std::string &token)
{
    cout << "Get All Open Orders..." << endl;

    return sendRequest("/api/v2/private/get_open_orders", {}, "GET", token);
}


// Function to get the state of a specific order by its ID
json DeribitClient::getOrderState(const string &order_id, const string &token)
{
    json payload = {{"order_id", order_id}};

    return sendRequest("/api/v2/private/get_order_state", payload, "GET", token);
}

// Function to retrieve the order book for a specific instrument
json DeribitClient::getOrderBook(const string &symbol)
{
    json payload = {{"instrument_name", symbol}};

    return sendRequest("/api/v2/public/get_order_book", payload, "GET");
}