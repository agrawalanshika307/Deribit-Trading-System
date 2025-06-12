#include "APIClient.h"  // API client for interacting with Deribit
#include <iostream>       // Standard input-output stream
#include <string>         // String handling
#include <thread>         // Threading support
#include <nlohmann/json.hpp>  // JSON handling library
#include "WebSocketClient.h" // WebSocket client for real-time data
#include "Utils.h"           // Utility functions

// Use JSON namespace for convenience
using json = nlohmann::json;
using namespace std;

int main()
{
    cout << "Starting Deribit Client..." << endl;
    
    // Create WebSocket client instance
    WebSocketClient wsClient; 
    
    // Create Deribit API client instance
    DeribitClient client;

    // Authenticate user and get access token
    json token = client.getAuthToken();
    if (token.empty())
    {
        cerr << "Authentication failed. Exiting...\n";
        return 1; // Exit the program if authentication fails
    }
    cout << "Authentication Done..." << endl;

    // Store the authentication token as a string
    string accessToken = token; 

    int ch; // Variable to store user input for menu selection
    do
    {
        displayMenu(); // Display available options to the user
        cin >> ch;     // Read user choice
        switch (ch)
        {
        case 1:
        {
            // Place a new order
            string instrument, type, label;
            double amount, price = 0.0;
            
            // Get order details from user
            cout << "Enter instrument name: ";
            cin >> instrument;
            cout << "Enter order type (limit/market): ";
            cin >> type;
            cout << "Enter amount: ";
            cin >> amount;
            
            // If order is limit type, ask for price
            if (type == "limit" || type == "stop_limit")
            {
                cout << "Enter price (Note that amount should be multiple of price): ";
                cin >> price;
            }
            
            // Measure execution time (latency) of placing an order
            auto start_time = std::chrono::high_resolution_clock::now();
            json response = client.placeOrder(accessToken, instrument, type, amount, price);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            
            // Display order response and execution latency
            cout << "Response: " << response.dump(4) << endl;
            cout << "Latency: " << latency << " msec" << endl;
            break;
        }
        case 2:
        {
            // Modify an existing order
            string order_id;
            double amount = 0, price = 0;
            
            // Get modification details from user
            cout << "Enter order ID: ";
            cin >> order_id;
            cout << "Enter new amount: ";
            cin >> amount;
            cout << "Enter new price (or 0 for market order): ";
            cin >> price;
            break;
        }
        case 3:
        {
            // Cancel an existing order
            string order_id;
            cout << "Enter order ID to cancel: ";
            cin >> order_id;
            
            // Send cancellation request
            json response = client.cancelOrder(order_id, accessToken);
            cout << "Response: " << response.dump(4) << endl;
            break;
        }
        case 4:
        {
            // Retrieve open orders
            json response = client.getOpenOrder(accessToken);
            cout << "Open Orders: " << response.dump(4) << endl;
            break;
        }
        case 5:
        {
            // Get order state by ID
            string order_id;
            cout << "Enter order ID to get state: ";
            cin >> order_id;
            json response = client.getOrderState(order_id, accessToken);
            cout << "Order State: " << response.dump(4) << endl;
            break;
        }
        case 6:
        {
            // Get order book for a given symbol
            string symbol;
            cout << "Enter symbol name: ";
            cin >> symbol;
            json response = client.getOrderBook(symbol);
            cout << "Order Book: " << response.dump(4) << endl;
            break;
        }
        case 7:
        {
            // Place a sell order
            string instrument, type;
            double amount, price = 0.0;
            
            // Get order details from user
            cout << "Enter instrument name: ";
            cin >> instrument;
            cout << "Enter order type (limit/market): ";
            cin >> type;
            cout << "Enter amount: ";
            cin >> amount;
            
            // If order is limit type, ask for price
            if (type == "limit" || type == "stop_limit")
            {
                cout << "Enter price (Note that amount should be multiple of price): ";
                cin >> price;
            }
            
            // Send sell order request
            json response = client.sellOrder(accessToken, instrument, amount, {}, price, type, {}, {});
            cout << "Response: " << response.dump(4) << endl;
            break;
        }
        case 8:
        {
            // Start WebSocket session in a separate thread
            std::thread webSocketThread(startWebSocketSession, std::ref(accessToken));
            webSocketThread.join(); // Wait for WebSocket thread to complete
            break;
        }
        }
    } while (ch != 9); // Continue execution until user chooses to exit

    return 0; // End program execution
}
