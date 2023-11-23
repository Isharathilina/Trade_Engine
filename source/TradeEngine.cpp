#include <iostream>
#include <map>
#include <queue>
#include <ctime>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>
#include <fstream>
#include <sstream>

#define SERVER_PORT 5051
#define SERVER_PORT_SEND_DATA 5052
using namespace std;

// Order structure
struct Order {
    string id;
    size_t count_id;
    string type;
    double price;
    int quantity;
    time_t timestamp;
};

// Trade structure
struct Trade {
    string buyOrderId;
    string sellOrderId;
    double price;
    int quantity;
    time_t timestamp;
};

// Global variables
map<int, Order> orderBook;  // Stores pending orders
queue<Trade> tradeQueue;  // Stores matched trades
ofstream logFile_tq("Completed_Trade_Data.log", ios::app); // complete order details log
size_t orderIdCounter = 1; // Unique incremental index for each order
bool sendTradeDataFlag = false; // falg for identify Trade data receiver client connect or not 

// Function to match orders
void matchOrders() 
{
    cout << "Starting matchOrders" << endl;
    while (true) 
    {
        if (orderBook.empty()) {
            continue;
        }
        // Find the first order in the order book
        auto firstOrderIt_f = orderBook.begin();
        auto firstOrder = firstOrderIt_f->second;
        // iterator for see map reverse order
        auto firstOrderIt = orderBook.rbegin();
        
        // Check if the order is a buy order
        if (firstOrder.type == "buy") 
        {
            // Check all subsequent sell orders
            for (auto sellOrderIt = firstOrderIt; sellOrderIt != prev(orderBook.rend()); sellOrderIt++) 
            {
                auto sellOrder = sellOrderIt->second;

                // Match orders if the price and quantity match
                if (sellOrder.type == "sell" && sellOrder.price == firstOrder.price 
                    && sellOrder.quantity == firstOrder.quantity) 
                {
                    // Match orders
                    int tradeQuantity = sellOrder.quantity;
                    Trade trade;
                    trade.buyOrderId = firstOrder.id;
                    trade.sellOrderId = sellOrder.id;
                    trade.price = firstOrder.price;
                    trade.quantity = tradeQuantity;
                    trade.timestamp = time(nullptr);

                    tradeQueue.push(trade);
                    // Remove the first order from the order book
                    orderBook.erase(firstOrder.count_id);
                    orderBook.erase(sellOrder.count_id);
                    cout<<endl<<"Matched with same price and quantity, Buy ID:-" <<trade.buyOrderId<<"  Sell ID:"<<trade.sellOrderId<< endl;
                    break;
                }else if (sellOrder.type == "sell" && sellOrder.price == firstOrder.price 
                          && sellOrder.quantity > firstOrder.quantity)
                {
                     // Match orders
                    int tradeQuantity = firstOrder.quantity;
                    Trade trade;
                    trade.buyOrderId = firstOrder.id;
                    trade.sellOrderId = sellOrder.id;
                    trade.price = firstOrder.price;
                    trade.quantity = tradeQuantity;
                    trade.timestamp = time(nullptr);

                    tradeQueue.push(trade);
                    // Remove the first order from the order book
                    orderBook.erase(firstOrder.count_id);
                    // update remaining quantitiy
                    orderBook[sellOrder.count_id].quantity -=tradeQuantity;
                    if (sellOrder.quantity == 0)
                    {
                        orderBook.erase(sellOrder.count_id);
                    } 
                    cout<<endl<<"Partial Matched with same price Buy ID:-" <<trade.buyOrderId<<" (completed)  Sell ID:"
                    <<trade.sellOrderId<<" (partial) Remaining Quantity:"<<orderBook[sellOrder.count_id].quantity<<endl;   
                    break;

                }else if (sellOrder.type == "sell" && sellOrder.price == firstOrder.price 
                          && sellOrder.quantity < firstOrder.quantity)
                {
                     // Match orders
                    int tradeQuantity = sellOrder.quantity;
                    Trade trade;
                    trade.buyOrderId = firstOrder.id;
                    trade.sellOrderId = sellOrder.id;
                    trade.price = sellOrder.price;
                    trade.quantity = tradeQuantity;
                    trade.timestamp = time(nullptr);

                    tradeQueue.push(trade);
                    // Remove the first order from the order book
                    orderBook.erase(sellOrder.count_id);
                    // update remaining quantitiy
                    orderBook[firstOrder.count_id].quantity -=tradeQuantity;
                    if (firstOrder.quantity == 0){
                        orderBook.erase(firstOrder.count_id);
                    }    
                    cout<<endl<<"Partial Matched with same price Buy ID:-" <<trade.buyOrderId<<" (partial) Remaining Quantity:"
                    <<orderBook[firstOrder.count_id].quantity<<"  Sell ID:"<<trade.sellOrderId<<" (completed)"<<endl;
                    break;
                }
            }

        }else  // the order should be a sell order
        {
            // Check all subsequent sell orders
            for (auto buyOrderIt = firstOrderIt; buyOrderIt != prev(orderBook.rend()); buyOrderIt++) 
            {
                auto buyOrder = buyOrderIt->second;

                // Match orders if the price and quantity match
                if (buyOrder.type == "buy" && buyOrder.price == firstOrder.price 
                    && buyOrder.quantity == firstOrder.quantity) 
                {
                    // Match orders
                    int tradeQuantity = buyOrder.quantity;
                    Trade trade;
                    trade.buyOrderId = buyOrder.id;
                    trade.sellOrderId = firstOrder.id;
                    trade.price = firstOrder.price;
                    trade.quantity = tradeQuantity;
                    trade.timestamp = time(nullptr);

                    tradeQueue.push(trade);
                    // Remove the first order from the order book
                    orderBook.erase(firstOrder.count_id);
                    orderBook.erase(buyOrder.count_id);
                    cout<<endl<<"Matched with same price and quantity, Buy ID:-" <<trade.buyOrderId<<"  Sell ID:"<<trade.sellOrderId<< endl;
                    break;
   
                }else if (buyOrder.type == "buy" && buyOrder.price == firstOrder.price 
                          && buyOrder.quantity > firstOrder.quantity)
                {
                     // Match orders
                    int tradeQuantity = firstOrder.quantity;
                    Trade trade;
                    trade.buyOrderId = buyOrder.id;
                    trade.sellOrderId = firstOrder.id;
                    trade.price = firstOrder.price;
                    trade.quantity = tradeQuantity;
                    trade.timestamp = time(nullptr);

                    tradeQueue.push(trade);
                    // Remove the first order from the order book
                    orderBook.erase(firstOrder.count_id);
                    // update remaining quantitiy
                    orderBook[buyOrder.count_id].quantity -=tradeQuantity;
                    if (buyOrder.quantity == 0){
                        orderBook.erase(buyOrder.count_id);
                    }
                    cout<<endl<<"Partial Matched with same price Buy ID:" <<trade.buyOrderId<<" (partial) Remaining Quantity:"
                    <<orderBook[buyOrder.count_id].quantity<<"   Sell ID:"<<trade.sellOrderId<<" (completed)"<<endl;   
                    break;

                }else if (buyOrder.type == "buy" && buyOrder.price == firstOrder.price 
                          && buyOrder.quantity < firstOrder.quantity)
                {
                     // Match orders
                    int tradeQuantity = buyOrder.quantity;
                    Trade trade;
                    trade.buyOrderId = buyOrder.id;
                    trade.sellOrderId = firstOrder.id;
                    trade.price = buyOrder.price;
                    trade.quantity = tradeQuantity;
                    trade.timestamp = time(nullptr);

                    tradeQueue.push(trade);
                    // Remove the first order from the order book
                    orderBook.erase(buyOrder.count_id);
                    // update remaining quantitiy
                    orderBook[firstOrder.count_id].quantity -=tradeQuantity;
                    if (firstOrder.quantity == 0){
                        orderBook.erase(firstOrder.count_id);
                    }
                    cout<<endl<<"Partial Matched with same price Buy ID:"<<trade.buyOrderId<<" (completed)  Sell ID:"
                    <<trade.sellOrderId<<" (partial) Remaining Quantity:"<<orderBook[buyOrder.count_id].quantity<<endl;   
                    break;
                }
            }
        }

    }
}

// Function to handle trades
void handleTrades() 
{
    cout<<"Starting handleTrades"<<endl;

    int serverSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    char buffer[1024];

    // Create a TCP/IP socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        close(serverSocket);
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    memset(serverAddress.sin_zero, 0, sizeof(serverAddress.sin_zero));

    // Bind the socket to the specified IP and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Socket binding failed");
        close(serverSocket);
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1) == -1) {
        perror("Socket listening failed");
        close(serverSocket);
        exit(1);
    }

    // Accept incoming connections
    cout << "Sockect Created, listining On Port : "<<SERVER_PORT<< endl;
    int clientSocket;
    while (true) 
    {
        // Accept a client connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize)) == -1) {
            perror("Socket accepting failed");
            continue;
        }

        // Receive trades from the client
        while (true) 
        {
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0)
                break;

            // Process the received trade
            string orderStr(buffer, bytesRead);
            //cout << "Received Order String: " << orderStr;

            //Extract Order and update the order book
            string delimiter = ",";
            size_t pos = 0;
            string token;
            string orderRefId;
            string type;
            double price;
            int quantity;

            // Extract orderId
            pos = orderStr.find(delimiter);
            token = orderStr.substr(0, pos);
            orderRefId = token;
            orderStr.erase(0, pos + delimiter.length());

            // Extract type
            pos = orderStr.find(delimiter);
            token = orderStr.substr(0, pos);
            type = token;
            orderStr.erase(0, pos + delimiter.length());

            // Extract price
            pos = orderStr.find(delimiter);
            token = orderStr.substr(0, pos);
            price = stod(token);
            orderStr.erase(0, pos + delimiter.length());

            // Extract quantity
            pos = orderStr.find(delimiter);
            token = orderStr.substr(0, pos);
            quantity = stoi(token);

            // Create a new order object
            Order order;
            order.count_id = orderIdCounter++;
            order.id = orderRefId;
            order.type = type;
            order.price = price;
            order.quantity = quantity;
            order.timestamp = time(nullptr);

            // Push the order into the order book
            orderBook[order.count_id] = order;
            cout<< "Order Book Size: "<< orderBook.size() <<" Extrated Order Details:" <<"ID = "<<order.id 
            << ", Type = " << order.type << ", Price = " << order.price << ", Quantity = " << order.quantity << endl;
        }

        close(clientSocket);
    }

    close(serverSocket);
}

// Function to monitor and display order book and trade queue count status
void offlineMonitorStatus() 
{
    size_t totalCompletedTrade = 0;  // Keep track of the last printed offlile trade ID
    while (true) 
    {
        if(!sendTradeDataFlag) // check online data sending ok
        {
            // Print newly added trades since the last print
            size_t queueSize = tradeQueue.size();
            totalCompletedTrade += queueSize;
            logFile_tq << "Total completed Trades: " << totalCompletedTrade << endl;
            logFile_tq << "Total New completed Trade Queue Size: " << queueSize << endl;
            logFile_tq << "-------------------------------------" << endl;
            for (size_t i = 0; i < queueSize; i++) {
                Trade trade = tradeQueue.front();
                logFile_tq << "Trade Details: Buy Order ID = " << trade.buyOrderId << ", Sell Order ID = " << trade.sellOrderId << ", Price = " << trade.price << ", Quantity = " << trade.quantity << endl;
                tradeQueue.pop();
            }
            this_thread::sleep_for(chrono::seconds(3));
        }

    }
}


void onlineTradeDataSender() 
{
    int logServerSocket;
    struct sockaddr_in logServerAddress, clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);

    // Create a TCP/IP socket
    if ((logServerSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Log server socket creation failed");
        return;
    }

    logServerAddress.sin_family = AF_INET;
    logServerAddress.sin_port = htons(SERVER_PORT_SEND_DATA);
    logServerAddress.sin_addr.s_addr = INADDR_ANY;
    memset(logServerAddress.sin_zero, 0, sizeof(logServerAddress.sin_zero));

    // Bind the socket to the specified IP and port
    if (bind(logServerSocket, (struct sockaddr*)&logServerAddress, sizeof(logServerAddress)) == -1) {
        perror("Log server socket binding failed");
        return;
    }

    // Listen for incoming connections
    if (listen(logServerSocket, 1) == -1) {
        perror("Log server socket listening failed");
        return;
    }

    // Accept incoming connections and send log data to clients
    cout << "Sockect Created, listining for Send Trade DATA On Port : "<<SERVER_PORT_SEND_DATA<< endl;
    int clientSocket;

    while (true) 
    {
        // Accept a client connection
        if ((clientSocket = accept(logServerSocket, (struct sockaddr*)&clientAddress, &clientAddressSize)) == -1) {
            perror("Log server socket accepting failed");
            continue;
        }
        //desable offline logs
        sendTradeDataFlag = true;

        // Send log data to the client application from tradeQueue
        while (true)
        {
            if (!tradeQueue.empty()) 
            {
                Trade trade = tradeQueue.front();
                stringstream ss;
                ss << "Trade Details: Buy Order ID = " << trade.buyOrderId << ", Sell Order ID = " << trade.sellOrderId << ", Price = " << trade.price << ", Quantity = " << trade.quantity << endl;
                string logData = ss.str();
                send(clientSocket, logData.c_str(), logData.size(), 0);
                tradeQueue.pop();
            }
        }            
    }
    close(clientSocket);
    close(logServerSocket);
}


int main() 
{
    if (!logFile_tq) {
        cerr << "Failed to open log file." << endl;
        return 1;
    }
    cout << "Starting Engine.."<< endl;
    logFile_tq << "Starting Engine.." << endl;
    // Matching engine
    thread matchThread(matchOrders);
    // Order acceptance thread
    thread tradeThread(handleTrades);
    // offline monitor thread
    thread offlineStatusThread(offlineMonitorStatus); 
    // online trade data sender 
    thread onlineDataSenderThread(onlineTradeDataSender);

    matchThread.join();
    tradeThread.join();
    offlineStatusThread.join();
    onlineDataSenderThread.join();

    return 0;
}
