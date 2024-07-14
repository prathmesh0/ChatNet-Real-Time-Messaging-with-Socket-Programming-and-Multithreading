#include <iostream>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

class ChatClient {
public:
    ChatClient(const string& serverAddress, int port) : serverAddress(serverAddress), port(port), socket(INVALID_SOCKET), running(true) {}

    ~ChatClient() {
        closesocket(socket);
        WSACleanup();
    }

    bool Initialize() {
        WSADATA data;
        int wsResult = WSAStartup(MAKEWORD(2, 2), &data);
        if (wsResult != 0) {
            cerr << "WSAStartup failed with error: " << wsResult << endl;
            return false;
        }
        return true;
    }

    bool Connect() {
        socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (socket == INVALID_SOCKET) {
            cerr << "Invalid socket created with error: " << WSAGetLastError() << endl;
            WSACleanup();
            return false;
        }

        sockaddr_in serveraddr;
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_port = htons(port);
        inet_pton(AF_INET, serverAddress.c_str(), &serveraddr.sin_addr);

        int connResult = connect(socket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));
        if (connResult == SOCKET_ERROR) {
            cerr << "Not able to connect to server with error: " << WSAGetLastError() << endl;
            closesocket(socket);
            WSACleanup();
            return false;
        }

        cout << "Successfully connected to the server" << endl;
        return true;
    }

    void Start() {
        thread senderThread(&ChatClient::SendMsg, this);
        thread receiverThread(&ChatClient::ReceiveMsg, this);

        senderThread.join();
        receiverThread.join();
    }

private:
    void SendMsg() {
        string name;
        cout << "Enter Your chat name: ";
        getline(cin, name);

        string message;
        while (running) {
            
            getline(cin, message);
            if (message == "quit") {
                cout << "Stopping the application..." << endl;
                running = false;
                break;
            }

            string msg = name + " : " + message;
            int bytesSent = send(socket, msg.c_str(), msg.length(), 0);
            if (bytesSent == SOCKET_ERROR) {
                cerr << "Error in sending message. Error: " << WSAGetLastError() << endl;
                running = false;
                break;
            }
        }
    }

    void ReceiveMsg() {
        char buffer[4096];
        while (running) {
            int bytesReceived = recv(socket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                cerr << "Disconnected from server or error occurred. Error: " << WSAGetLastError() << endl;
                running = false;
                break;
            }
            string msg(buffer, bytesReceived);
            cout << msg << endl;
        }
    }

    string serverAddress;
    int port;
    SOCKET socket;
    atomic<bool> running;
};

int main() {
    ChatClient client("127.0.0.1", 8000);
    if (!client.Initialize()) {
        cerr << "Initialize winsock failed" << endl;
        return 1;
    }

    if (!client.Connect()) {
        return 1;
    }

    client.Start();

    return 0;
}
