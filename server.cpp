#include <iostream>
#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Handles one connected client
void handleClient(
    SOCKET clientSocket,
    unordered_map<string, string> &db,
    mutex &dbMutex)
{
    char buffer[1024];
    string inputBuffer;

    while (true)
    {
        int bytesReceived = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0);

        if (bytesReceived <= 0)
        {
            break;
        }

        // Add received bytes to our persistent buffer
        inputBuffer.append(buffer, bytesReceived);

        // Process every complete command
        while (true)
        {
            size_t newlinePosition = inputBuffer.find('\n');

            // No complete command yet
            if (newlinePosition == string::npos)
            {
                break;
            }

            // Extract one complete command
            string command =
                inputBuffer.substr(0, newlinePosition);

            // Remove command + '\n' from buffer
            inputBuffer.erase(
                0,
                newlinePosition + 1);

            cout << "Command: "
                 << command
                 << endl;

            // Parse command
            stringstream ss(command);

            string operation;
            string key;
            string value;
            string something;

            ss >> operation;

            // =========================
            // PING
            // =========================

            if (operation == "PING")
            {
                if ((ss >> something))
                {
                    string response = "ERROR\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0);
                }
                else
                {
                    string response = "PONG\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0);
                }
            }

            // =========================
            // SET
            // =========================

            else if (operation == "SET")
            {
                if (!(ss >> key) || !(ss >> value) || (ss >> something))
                {
                    string response = "ERROR\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0);
                }
                else

                {
                    lock_guard<mutex> lock(dbMutex);

                    db[key] = value;

                    string response = "OK\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0);
                }
            }

            // =========================
            // GET
            // =========================

            else if (operation == "GET")
            {

                if (!(ss >> key)|| (ss >> something))
                {
                    string response = "ERROR\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0);
                }
                else
                {
                    string response;

                    {
                        lock_guard<mutex> lock(dbMutex);

                        if (db.find(key) != db.end())
                        {
                            response = db[key];
                        }
                        else
                        {
                            response = "NOT_FOUND";
                        }
                    }

                    response += '\n';

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0);
                }
            }

            else if (operation == "DEL")
            {
                if (!(ss >> key))
                {
                    string response = "ERROR\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0);
                }
                else
                {

                    string response;

                    {
                        lock_guard<mutex> lock(dbMutex);

                        if (db.find(key) != db.end())
                        {
                            db.erase(key);
                            response = "OK";
                        }
                        else
                        {
                            response = "NOT_FOUND";
                        }
                    }

                    response += '\n';

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0);
                }
            }
            else if (operation == "EXISTS")
            {
                if (!(ss >> key))
                {
                    string response = "ERROR\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0);
                }
                else
                {
                    string response;

                    {
                        lock_guard<mutex> lock(dbMutex);

                        if (db.find(key) != db.end())
                        {
                            response = "1";
                        }
                        else
                        {
                            response = "0";
                        }
                    }

                    response += '\n';

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0);
                }
            }

            // =========================
            // UNKNOWN COMMAND
            // =========================

            else
            {
                string response = "UNKNOWN_COMMAND\n";

                send(
                    clientSocket,
                    response.c_str(),
                    response.size(),
                    0);
            }
        }
    }

    // Client disconnected
    closesocket(clientSocket);

    cout << "Client disconnected.\n";
}

int main()
{
    // Our shared key-value database
    unordered_map<string, string> db;
    mutex dbMutex;

    // ========================================
    // 1. Start Winsock
    // ========================================

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed\n";
        return 1;
    }

    // ========================================
    // 2. Create socket
    // ========================================

    SOCKET serverSocket = socket(
        AF_INET,
        SOCK_STREAM,
        0);

    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";

        WSACleanup();

        return 1;
    }

    cout << "Socket created successfully!\n";

    // ========================================
    // 3. Create server address
    // ========================================

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;

    serverAddress.sin_port = htons(5000);

    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // ========================================
    // 4. Bind socket to IP + port
    // ========================================

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

    // ========================================
    // 5. Start listening
    // ========================================

    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        cout << "Listen failed\n";

        closesocket(serverSocket);

        WSACleanup();

        return 1;
    }

    cout << "Server is listening on port 5000...\n";

    // ========================================
    // 6. Accept clients continuously
    // ========================================

    while (true)
    {
        SOCKET clientSocket = accept(
            serverSocket,
            nullptr,
            nullptr);

        if (clientSocket == INVALID_SOCKET)
        {
            cout << "Accept failed\n";

            continue;
        }

        cout << "Client connected!\n";

        // Create a new thread for this client
        thread clientThread(
            handleClient,
            clientSocket,
            ref(db), ref(dbMutex));

        // Let the thread run independently
        clientThread.detach();
    }

    // ========================================
    // Cleanup
    // ========================================

    closesocket(serverSocket);

    WSACleanup();

    cout << "Server stopped.\n";

    return 0;
}