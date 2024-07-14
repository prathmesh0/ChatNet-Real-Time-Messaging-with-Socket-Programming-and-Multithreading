#include <iostream>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool Initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

class ChatServer {
public:
    ChatServer(int port) : port(port), serverRunning(true) {
        Initialize();
        CreateListeningSocket();
        BindSocket();
        StartListening();
    }

    ~ChatServer() {
        Shutdown();
    }

    void Run() {
        AcceptConnections();
    }

private:
    int port;
    SOCKET listenSocket;
    vector<SOCKET> clients;
    mutex clientsMutex;
    atomic<bool> serverRunning;

    void CreateListeningSocket() {
        listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listenSocket == INVALID_SOCKET) {
            cerr << "Socket creation failed with error: " << WSAGetLastError() << endl;
            WSACleanup();
            exit(1);
        }
    }

    void BindSocket() {
        sockaddr_in serveraddr = { 0 };
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_port = htons(port);

        if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
            cerr << "Setting address structure failed with error: " << WSAGetLastError() << endl;
            closesocket(listenSocket);
            WSACleanup();
            exit(1);
        }

        if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
            cerr << "Binding failed with error: " << WSAGetLastError() << endl;
            closesocket(listenSocket);
            WSACleanup();
            exit(1);
        }
    }

    void StartListening() {
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            cerr << "Listen failed with error: " << WSAGetLastError() << endl;
            closesocket(listenSocket);
            WSACleanup();
            exit(1);
        }
        cout << "Server is listening on port " << port << endl;
    }

    void AcceptConnections() {
        while (serverRunning) {
            SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) {
                cerr << "Invalid client socket with error: " << WSAGetLastError() << endl;
                continue;
            }

            {
                lock_guard<mutex> lock(clientsMutex);
                clients.push_back(clientSocket);
            }

            thread(&ChatServer::HandleClient, this, clientSocket).detach();
        }
    }

    void HandleClient(SOCKET clientSocket) {
        cout << "Client connected" << endl;
        char buffer[4096];
        while (true) {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                cout << "Client disconnected" << endl;
                break;
            }
            string message(buffer, bytesReceived);
            cout << "Message from client: " << message << endl;
            BroadcastMessage(clientSocket, message);
        }

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.erase(remove(clients.begin(), clients.end(), clientSocket), clients.end());
        }

        closesocket(clientSocket);
    }

    void BroadcastMessage(SOCKET sender, const string& message) {
        lock_guard<mutex> lock(clientsMutex);
        for (const auto& client : clients) {
            if (client != sender) {
                send(client, message.c_str(), message.length(), 0);
            }
        }
    }

    void Shutdown() {
        serverRunning = false;
        closesocket(listenSocket);
        WSACleanup();
        {
            lock_guard<mutex> lock(clientsMutex);
            for (const auto& client : clients) {
                closesocket(client);
            }
            clients.clear();
        }
    }
};

int main() {
    ChatServer server(8000);
    server.Run();
    return 0;
}
