#include <iostream>
#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;


// =====================================================
// SAVE DATABASE TO FILE
// =====================================================

void saveDatabase(unordered_map<string, string>& db)
{
    cout << "saveDatabase() called!" << endl;

    ofstream file("database.txt");

    if (!file.is_open())
    {
        cout << "ERROR: Could not open database.txt" << endl;
        return;
    }

    cout << "File opened successfully!" << endl;

    for (auto& pair : db)
    {
        cout << "Writing: "
             << pair.first
             << " = "
             << pair.second
             << endl;

        file << pair.first << " "
             << pair.second << "\n";
    }

    file.flush();

    file.close();

    cout << "Database saved!" << endl;
}


// =====================================================
// LOAD DATABASE FROM FILE
// =====================================================

void loadDatabase(unordered_map<string, string>& db)
{
    ifstream file("database.txt");

    if (!file.is_open())
    {
        cout << "No existing database found.\n";
        return;
    }

    string key;
    string value;

    while (file >> key >> value)
    {
        db[key] = value;
    }

    file.close();

    cout << "Database loaded!\n";
}


// =====================================================
// HANDLE ONE CLIENT
// =====================================================

void handleClient(
    SOCKET clientSocket,
    unordered_map<string, string>& db,
    mutex& dbMutex)
{
    char buffer[1024];

    string inputBuffer;


    while (true)
    {
        // =================================================
        // RECEIVE DATA
        // =================================================

        int bytesReceived = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
        );


        if (bytesReceived <= 0)
        {
            break;
        }


        // Add received bytes to persistent buffer

        inputBuffer.append(
            buffer,
            bytesReceived
        );


        // =================================================
        // PROCESS COMPLETE COMMANDS
        // =================================================

        while (true)
        {
            size_t newlinePosition =
                inputBuffer.find('\n');


            // Command is incomplete

            if (newlinePosition == string::npos)
            {
                break;
            }


            // Extract command

            string command =
                inputBuffer.substr(
                    0,
                    newlinePosition
                );


            // Remove command + newline

            inputBuffer.erase(
                0,
                newlinePosition + 1
            );


            cout << "Command: "
                 << command
                 << endl;


            // =================================================
            // PARSE COMMAND
            // =================================================

            stringstream ss(command);

            string operation;
            string key;
            string value;
            string something;


            ss >> operation;


            // =================================================
            // PING
            // =================================================

            if (operation == "PING")
            {
                if (ss >> something)
                {
                    string response = "ERROR\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0
                    );
                }
                else
                {
                    string response = "PONG\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0
                    );
                }
            }


            // =================================================
            // SET
            // =================================================

            else if (operation == "SET")
            {
                // SET requires:
                // SET key value

                if (!(ss >> key) ||
                    !(ss >> value) ||
                    (ss >> something))
                {
                    string response = "ERROR\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0
                    );
                }
                else
                {
                    {
                        lock_guard<mutex> lock(dbMutex);

                        db[key] = value;

                        // Save updated database
                        saveDatabase(db);
                    }


                    string response = "OK\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0
                    );
                }
            }


            // =================================================
            // GET
            // =================================================

            else if (operation == "GET")
            {
                // GET requires:
                // GET key

                if (!(ss >> key) ||
                    (ss >> something))
                {
                    string response = "ERROR\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0
                    );
                }
                else
                {
                    string response;


                    {
                        lock_guard<mutex> lock(dbMutex);


                        auto it = db.find(key);


                        if (it != db.end())
                        {
                            response = it->second;
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
                        0
                    );
                }
            }


            // =================================================
            // DEL
            // =================================================

            else if (operation == "DEL")
            {
                // DEL requires:
                // DEL key

                if (!(ss >> key) ||
                    (ss >> something))
                {
                    string response = "ERROR\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0
                    );
                }
                else
                {
                    string response;


                    {
                        lock_guard<mutex> lock(dbMutex);


                        auto it = db.find(key);


                        if (it != db.end())
                        {
                            db.erase(it);

                            // Save updated database
                            saveDatabase(db);

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
                        0
                    );
                }
            }


            // =================================================
            // EXISTS
            // =================================================

            else if (operation == "EXISTS")
            {
                // EXISTS requires:
                // EXISTS key

                if (!(ss >> key) ||
                    (ss >> something))
                {
                    string response = "ERROR\n";

                    send(
                        clientSocket,
                        response.c_str(),
                        response.size(),
                        0
                    );
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
                        0
                    );
                }
            }


            // =================================================
            // UNKNOWN COMMAND
            // =================================================

            else
            {
                string response =
                    "UNKNOWN_COMMAND\n";


                send(
                    clientSocket,
                    response.c_str(),
                    response.size(),
                    0
                );
            }
        }
    }


    // =================================================
    // CLIENT DISCONNECTED
    // =================================================

    closesocket(clientSocket);

    cout << "Client disconnected.\n";
}


// =====================================================
// MAIN
// =====================================================

int main()
{
    // =================================================
    // OUR SHARED KEY-VALUE DATABASE
    // =================================================

    unordered_map<string, string> db;

    mutex dbMutex;


    // =================================================
    // LOAD DATABASE FROM DISK
    // =================================================

    loadDatabase(db);


    // =================================================
    // 1. START WINSOCK
    // =================================================

    WSADATA wsaData;


    if (WSAStartup(
            MAKEWORD(2, 2),
            &wsaData) != 0)
    {
        cout << "WSAStartup failed\n";

        return 1;
    }


    // =================================================
    // 2. CREATE SERVER SOCKET
    // =================================================

    SOCKET serverSocket =
        socket(
            AF_INET,
            SOCK_STREAM,
            0
        );


    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";

        WSACleanup();

        return 1;
    }


    cout << "Socket created successfully!\n";


    // =================================================
    // 3. SERVER ADDRESS
    // =================================================

    sockaddr_in serverAddress{};


    serverAddress.sin_family =
        AF_INET;


    serverAddress.sin_port =
        htons(5000);


    serverAddress.sin_addr.s_addr =
        INADDR_ANY;


    // =================================================
    // 4. BIND
    // =================================================

    if (bind(
            serverSocket,
            (sockaddr*)&serverAddress,
            sizeof(serverAddress))
        == SOCKET_ERROR)
    {
        cout << "Bind failed\n";

        closesocket(serverSocket);

        WSACleanup();

        return 1;
    }


    cout << "Server bound to port 5000\n";


    // =================================================
    // 5. LISTEN
    // =================================================

    if (listen(
            serverSocket,
            5)
        == SOCKET_ERROR)
    {
        cout << "Listen failed\n";

        closesocket(serverSocket);

        WSACleanup();

        return 1;
    }


    cout << "Server is listening on port 5000...\n";


    // =================================================
    // 6. ACCEPT CLIENTS
    // =================================================

    while (true)
    {
        SOCKET clientSocket =
            accept(
                serverSocket,
                nullptr,
                nullptr
            );


        if (clientSocket == INVALID_SOCKET)
        {
            cout << "Accept failed\n";

            continue;
        }


        cout << "Client connected!\n";


        // Create thread for client

        thread clientThread(
            handleClient,
            clientSocket,
            ref(db),
            ref(dbMutex)
        );


        // Let thread run independently

        clientThread.detach();
    }


    // =================================================
    // CLEANUP
    // =================================================

    closesocket(serverSocket);

    WSACleanup();


    cout << "Server stopped.\n";


    return 0;
}