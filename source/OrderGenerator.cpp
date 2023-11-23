#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1" // TradeEngine IP
#define SERVER_PORT 5051      // TradeEngine PORT
#define MSG_INTERVEL 100      // set order sending interval (milliseconds)

using namespace std;

void generateOrders() 
{
    srand(time(nullptr));

    int clientSocket;
    struct sockaddr_in serverAddress;

    // Create a TCP/IP socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    memset(serverAddress.sin_zero, 0, sizeof(serverAddress.sin_zero));

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Connection failed");
        return;
    }

    // Generate random orders and send them over the TCP/IP connection
    while (true) 
    {
        int orderId = rand() % 10000 + 1;
        string orderType = (rand() % 2 == 0) ? "buy" : "sell";
        double orderPrice = static_cast<double>(rand() % 100) + 1.0;
        int orderQuantity = rand() % 10 + 1;

        string orderStr = to_string(orderId) + "," + orderType + ","
            + to_string(orderPrice) + "," + to_string(orderQuantity) + "\n";

        // Send the order over the TCP/IP connection
        if (send(clientSocket, orderStr.c_str(), orderStr.length(), 0) == -1) {
            perror("Send failed");
            break;
        }

        cout << "Order sent:"<<orderStr<<endl;
        this_thread::sleep_for(chrono::milliseconds(MSG_INTERVEL));  // Simulate high-speed trading
        
    }

    close(clientSocket);
}

int main() 
{
    generateOrders();
    return 0;
}
