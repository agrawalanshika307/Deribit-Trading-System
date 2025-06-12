#ifndef WEBSOCKETCLIENT_H 
#define WEBSOCKETCLIENT_H

// Standard C++ headers
#include <iostream>      // Standard input-output stream
#include <string>        // String handling
#include <thread>        // Thread support for multi-threading
#include <mutex>         // Mutex for thread synchronization
#include <chrono>        // Time-related functionalities

// Boost headers for networking, SSL, and WebSocket communication
#include <boost/beast/core.hpp>         // Core functionalities of Boost.Beast
#include <boost/asio.hpp>               // Boost.Asio for network programming
#include <boost/asio/ssl.hpp>           // SSL/TLS support in Boost.Asio
#include <boost/asio/connect.hpp>       // Connection handling for Boost.Asio
#include <boost/algorithm/string.hpp>   // String manipulation utilities
#include <nlohmann/json.hpp>            // JSON parsing and handling
#include <boost/beast/websocket/stream.hpp> // WebSocket stream support
#include <boost/asio/ssl/stream.hpp>    // SSL stream handling
#include <boost/asio/ip/tcp.hpp>        // TCP protocol handling
#include <boost/beast/websocket.hpp>    // WebSocket support
#include <boost/asio/ssl/stream.hpp>    // SSL stream for encrypted communication

// Namespace aliases to simplify usage of Boost libraries
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using json = nlohmann::json;
using namespace std;
using namespace std::chrono;
using tcp = boost::asio::ip::tcp;

// Define aliases for SSL and WebSocket streams
using ssl_stream = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
using websocket_stream = boost::beast::websocket::stream<ssl_stream>;

/**
 * @class WebSocketClient
 * @brief Handles WebSocket communication with an SSL-secured server.
 */
class WebSocketClient {
public:
    /**
     * @brief Constructs a WebSocketClient object.
     * Initializes the resolver, WebSocket stream, and SSL context.
     */
    WebSocketClient();

    /**
     * @brief Destroys the WebSocketClient object.
     * Ensures that the WebSocket connection is properly closed before destruction.
     */
    ~WebSocketClient();

    /**
     * @brief Establishes a connection to the WebSocket server.
     * @param host The WebSocket server address.
     * @param port The port number to connect to.
     */
    void connect(const std::string& host, const std::string& port);

    /**
     * @brief Subscribes to a specific WebSocket channel.
     * @param subscription The channel name (e.g., market data feed).
     * @param accessToken The authentication token required for private subscriptions.
     */
    void subscribe(const std::string& subscription, const std::string& accessToken);

    /**
     * @brief Listens for incoming messages from the WebSocket server.
     * Continuously reads messages and processes them in a separate thread.
     */
    void listen();

    /**
     * @brief Closes the WebSocket connection gracefully.
     */
    void close();

private:
    net::io_context ioc;                ///< Boost.Asio IO context for managing asynchronous operations
    ssl::context ssl_ctx{ssl::context::tlsv12_client}; ///< SSL context for secure communication
    tcp::resolver resolver;             ///< Resolves domain names to IP addresses
    websocket_stream ws;                ///< WebSocket stream for communication
    std::mutex mutex_;                   ///< Mutex for synchronizing output and shared resources
};

/**
 * @brief Starts a WebSocket session by connecting and subscribing to data streams.
 * @param token Authentication token required for private WebSocket subscriptions.
 */
void startWebSocketSession(std::string& token);

#endif // WEBSOCKETCLIENT_H
