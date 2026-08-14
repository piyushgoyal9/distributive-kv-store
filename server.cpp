#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
    unordered_map<string, string> db;
    // 1. Start Winsock
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed\n";
        return 1;
    }

    // 2. Create socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    cout << "Socket created successfully!\n";

    // 3. Create server address
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // 4. Bind socket to IP + port
    if (bind(
            serverSocket,
            (sockaddr *)&serverAddress,
            sizeof(serverAddress)) == SOCKET_ERROR)
    {

        cout << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server bound to port 5000\n";

    // 5. Start listening
    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {

        cout << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on port 5000...\n";

    // 6. Accept a client
    SOCKET clientSocket = accept(
        serverSocket,
        nullptr,
        nullptr);

    if (clientSocket == INVALID_SOCKET)
    {

        cout << "Accept failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Client connected!\n";

    // 7. Receive data
    char buffer[1024];
    while (true)
    {

        int bytesReceived = recv(
            clientSocket,
            buffer,
            sizeof(buffer) - 1,
            0);

        if (bytesReceived <= 0)
        {
            break;
        }

        buffer[bytesReceived] = '\0';

        cout << "Client said: "
             << buffer
             << endl;

        // 8. Send response
        string command(buffer);

        stringstream ss(command);

        string operation;
        string key;
        string value;

        ss >> operation;

        if (operation == "PING")
        {

            string response = "PONG";

            send(
                clientSocket,
                response.c_str(),
                response.size(),
                0);
        }
        else if (operation == "SET")
        {

            ss >> key;
            ss >> value;

            db[key] = value;

            string response = "OK";

            send(
                clientSocket,
                response.c_str(),
                response.size(),
                0);
        }
        else if (operation == "GET")
        {

            ss >> key;

            if (db.find(key) != db.end())
            {

                string response = db[key];

                send(
                    clientSocket,
                    response.c_str(),
                    response.size(),
                    0);
            }
            else
            {

                string response = "NOT_FOUND";

                send(
                    clientSocket,
                    response.c_str(),
                    response.size(),
                    0);
            }
        }
    }

    // 9. Close client connection
    closesocket(clientSocket);

    // 10. Close server socket
    closesocket(serverSocket);

    // 11. Cleanup Winsock
    WSACleanup();

    cout << "Server stopped.\n";

    return 0;
}