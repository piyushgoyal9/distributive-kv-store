#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed\n";
        return 1;
    }

    // Create socket
    SOCKET clientSocket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    cout << "Client socket created!\n";


    // Server address
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5000);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &serverAddress.sin_addr
    );


    // Connect
    if (connect(
            clientSocket,
            (sockaddr*)&serverAddress,
            sizeof(serverAddress)
        ) == SOCKET_ERROR)
    {
        cout << "Connection failed\n";

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }

    cout << "Connected to server!\n";


    // =====================================
    // SEND MULTIPLE COMMANDS IN ONE SEND
    // =====================================

    string commands =
        "SET name Piyush\n"
        "GET name\n"
        "PING\n";


    cout << "\nSending all commands in ONE send():\n";
    cout << commands << endl;


    send(
        clientSocket,
        commands.c_str(),
        commands.size(),
        0
    );


    // =====================================
    // Receive responses
    // =====================================

    char buffer[1024];

    int bytesReceived = recv(
        clientSocket,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';

        cout << "Server responses:\n";
        cout << buffer << endl;
    }


    closesocket(clientSocket);

    WSACleanup();

    return 0;
}