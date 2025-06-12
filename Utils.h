#ifndef UTILS_H 
#define UTILS_H

#include <iostream> // Include standard input-output library
using namespace std;

// Function to display the menu options to the user
void displayMenu() {
    cout << "\n ----Deribit Management System ----\n"; // Header for the menu
    cout << "1. Place Order\n";       // Option to place a new order
    cout << "2. Modify Order\n";      // Option to modify an existing order
    cout << "3. Cancel Order\n";      // Option to cancel an order
    cout << "4. Get Open Order\n";    // Option to retrieve all open orders
    cout << "5. Get Order State\n";   // Option to check the status of an order
    cout << "6. Get Order Book\n";    // Option to view the order book for a specific instrument
    cout << "7. Sell Order\n";        // Option to place a sell order
    cout << "8. Realtime Data\n";     // Option to access real-time market data
    cout << "9. Exit\n";              // Option to exit the program
    cout << "-------------------------------\n"; // Separator for clarity
    cout << "Enter your choice: ";    // Prompt user for input
}

#endif // End of header guard
