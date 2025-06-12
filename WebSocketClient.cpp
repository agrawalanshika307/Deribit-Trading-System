#include "WebSocketClient.h" // Include the WebSocketClient header file
#include <iostream>          // Standard I/O stream
#include <string>            // String handling
#include <thread>            // Threading support
#include <mutex>             // Mutex for thread safety
#include <chrono>            // Time-related functions
#include <boost/asio.hpp>    // Boost.Asio for network communication
#include <boost/asio/ssl.hpp> // Boost.Asio for SSL connections
#include <boost/asio/ip/tcp.hpp> // TCP support
#include <boost/asio/connect.hpp> // Connection handling
#include <boost/beast/websocket.hpp> // WebSocket functionality from Boost.Beast
#include <boost/algorithm/string.hpp> // String algorithms
#include <nlohmann/json.hpp> // JSON handling using the nlohmann library
#include <boost/beast/websocket/ssl.hpp> // SSL WebSocket support
#include <boost/beast/core.hpp> // Core functionality of Boost.Beast
#include <boost/asio/ssl/stream.hpp> // SSL stream
#include <boost/asio/ip/tcp.hpp> // TCP protocol support

// Aliases for frequently used namespaces from Boost and JSON libraries
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;
using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

// Define an alias for an SSL stream that operates over a TCP socket
using ssl_stream = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

// Function to properly close an SSL connection
void teardown(
    beast::role_type role,
    ssl_stream &stream,
    beast::error_code &ec)
{
    stream.shutdown(ec); // Gracefully shut down the SSL connection
}

// Constructor for the WebSocketClient class
WebSocketClient::WebSocketClient() : resolver(ioc), ws(ioc, ssl_ctx)
{
    ssl_ctx.set_default_verify_paths(); // Load default SSL certificates for verification
}

// Destructor for the WebSocketClient class
WebSocketClient::~WebSocketClient()
{
    if (ws.is_open()) // Check if the WebSocket connection is open
    {
        ws.close(websocket::close_code::normal); // Close the WebSocket connection gracefully
    }
}

// Function to establish a WebSocket connection
void WebSocketClient::connect(const std::string &host, const std::string &port)
{
    // Resolve the host and port to obtain a list of endpoints
    auto const results = resolver.resolve(host, port);

    // Connect to the resolved endpoint
    auto ep = net::connect(beast::get_lowest_layer(ws), results);

    // Configure SSL settings
    if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str()))
    {
        beast::error_code ec{
            static_cast<int>(::ERR_get_error()),
            net::error::get_ssl_category()};
        throw beast::system_error{ec}; // Throw an error if SSL handshake fails
    }

    // Perform SSL handshake
    ws.next_layer().handshake(ssl::stream_base::client);

    // Perform WebSocket handshake
    ws.handshake(host + ":" + port, "/ws/api/v2");
    std::cout << "WebSocket connected to " << host << " : " << port << std::endl;
}

// Function to subscribe to a specific WebSocket channel
void WebSocketClient::subscribe(const std::string &channel, const std::string &token)
{
    // Create a JSON request payload for subscription
    json payload = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "private/subscribe"},
        {"params", {{"access_token", token}, {"channels", {channel}}}}};

    // Send the subscription request via WebSocket
    ws.write(net::buffer(payload.dump()));
    std::cout << "Subscribed to channel: " << channel << std::endl;
}

// Function to close the WebSocket connection
void WebSocketClient::close()
{
    if (ws.is_open()) // Check if the WebSocket is still open
    {
        ws.close(websocket::close_code::normal); // Close the connection properly
    }
    std::cout << "WebSocket connection closed." << std::endl;
}

// Function to continuously listen for incoming WebSocket messages
void WebSocketClient::listen()
{
    try
    {
        while (true) // Infinite loop to keep listening for messages
        {
            beast::flat_buffer buffer; // Buffer to store incoming messages
            ws.read(buffer); // Read data from the WebSocket into the buffer

            // Convert buffer data to a string
            auto data = beast::buffers_to_string(buffer.data());
            json response = json::parse(data); // Parse the string into a JSON object

            // Check if the response contains timestamp data for latency measurement
            if (response.contains("params") && response["params"].contains("data") &&
                response["params"]["data"].contains("timestamp"))
            {
                auto server_time = response["params"]["data"]["timestamp"].get<long long>();
                auto client_time = duration_cast<milliseconds>(
                                       system_clock::now().time_since_epoch())
                                       .count();

                // Calculate the time delay between the server and client
                auto propagation_delay = client_time - server_time;
                std::cout << "Propagation delay: " << propagation_delay << " ms" << std::endl;
            }

            // Lock the mutex to prevent data race conditions while printing
            std::lock_guard<std::mutex> lock(mutex);
            std::cout << "Received update: " << response.dump(4) << std::endl;
        }
    }
    catch (const std::exception &e) // Catch any exceptions that occur
    {
        std::cerr << "Error during WebSocket read: " << e.what() << std::endl;
    }
}

// Function to start a WebSocket session and subscribe to market data
void startWebSocketSession(std::string &token)
{
    WebSocketClient wsClient; // Create an instance of WebSocketClient

    // Connect to Deribit's test WebSocket server on port 443 (SSL secured)
    wsClient.connect("test.deribit.com", "443");

    // Prompt the user for the instrument (symbol) they want to subscribe to
    std::cout << "Enter the instrument/symbol (e.g., BTC-PERPETUAL) you want to subscribe:\n";
    std::string symbol;
    std::cin >> symbol;

    // Prompt the user to choose a data interval
    std::cout << "Choose the interval:\n1. 100ms\n2. raw\n3. agg2\n";
    int intervalChoice;
    std::cin >> intervalChoice;

    // Construct the subscription string based on the chosen interval
    std::string subscription = "book." + symbol;
    if (intervalChoice == 1)
        subscription += ".100ms";
    else if (intervalChoice == 2)
        subscription += ".raw";
    else
        subscription += ".agg2";

    // Subscribe to the constructed channel using the provided token
    wsClient.subscribe(subscription, token);

    // Start a separate thread to continuously listen for WebSocket updates
    std::thread listener([&wsClient]()
                         { wsClient.listen(); });

    listener.join(); // Wait for the listener thread to finish execution
}
