#include <iostream>
#include <cstring>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

int main() {

    // 1. Start Winsock
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed\n";
        return 1;
    }


    // 2. Create client socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    cout << "Client socket created!\n";


    // 3. Server address
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5000);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &serverAddress.sin_addr
    );


    // 4. Connect to server
    if (connect(
        clientSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) == SOCKET_ERROR) {

        cout << "Connection failed\n";

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }

    cout << "Connected to server!\n";


    // 5. Keep sending commands
    while (true) {

        string command;

        cout << "> ";
        getline(cin, command);


        // Exit command
        if (command == "exit") {
            break;
        }


        // Send command
        send(
            clientSocket,
            command.c_str(),
            command.size(),
            0
        );


        // Receive response
        char buffer[1024];

        int bytesReceived = recv(
            clientSocket,
            buffer,
            sizeof(buffer) - 1,
            0
        );


        if (bytesReceived > 0) {

            buffer[bytesReceived] = '\0';

            cout << buffer << endl;

        } else {

            cout << "Server disconnected.\n";
            break;
        }
    }


    // 6. Close connection
    closesocket(clientSocket);

    WSACleanup();

    return 0;
}