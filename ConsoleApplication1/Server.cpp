#include "Server.h"
#include "NetworkProtocol.h"

// Link the Windows system library for network operations
#pragma comment(lib, "ws2_32.lib") 

Server::Server() {
    listenSocket = INVALID_SOCKET;
    player1Socket = INVALID_SOCKET;
    player2Socket = INVALID_SOCKET;
    isRunning = false;
}

Server::~Server() {
    Stop();
}

bool Server::Start(int port) {
    // 1. Winsock initialization
    WSADATA wsaData;
    int wsaStartupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaStartupResult != 0) {
        std::cerr << "Winsock initialization error: " << wsaStartupResult << std::endl;
        return false;
    }

    // 2. Create listening socket
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return false;
    }

    // 3. Configure address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // 4. Bind socket
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Socket bind error." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    // 5. Start listening
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Port listening error." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    std::cout << "Server successfully started on port " << port << ". Waiting for players..." << std::endl;
    isRunning = true;
    return true;
}

void Server::WaitForPlayers() {
    // Wait for first player
    player1Socket = accept(listenSocket, nullptr, nullptr);
    if (player1Socket == INVALID_SOCKET) {
        std::cerr << "Player 1 connection error!" << std::endl;
        return;
    }
    std::cout << "Player 1 connected! Waiting for Player 2..." << std::endl;

    // Wait for second player
    player2Socket = accept(listenSocket, nullptr, nullptr);
    if (player2Socket == INVALID_SOCKET) {
        std::cerr << "Player 2 connection error!" << std::endl;
        return;
    }
    std::cout << "Player 2 connected! Both players are ready." << std::endl;
}

void Server::GameLoop() {
    std::cout << "Packet forwarding started..." << std::endl;
    char buffer[512];

    while (isRunning) {
        // Check messages from Player 1
        int bytesAvailable = recv(player1Socket, buffer, sizeof(PacketHeader), MSG_PEEK);
        if (bytesAvailable > 0) {
            PacketHeader* header = (PacketHeader*)buffer;
            int bytesRead = recv(player1Socket, buffer, header->size, 0);
            send(player2Socket, buffer, bytesRead, 0); // Forward to Player 2
        }
        else if (bytesAvailable == 0) {
            std::cout << "Player 1 left the game." << std::endl;
            break;
        }

        // Check messages from Player 2
        bytesAvailable = recv(player2Socket, buffer, sizeof(PacketHeader), MSG_PEEK);
        if (bytesAvailable > 0) {
            PacketHeader* header = (PacketHeader*)buffer;
            int bytesRead = recv(player2Socket, buffer, header->size, 0);
            send(player1Socket, buffer, bytesRead, 0); // Forward to Player 1
        }
        else if (bytesAvailable == 0) {
            std::cout << "Player 2 left the game." << std::endl;
            break;
        }

        Sleep(10); // Prevent the server from using 100% CPU
    }
}

void Server::Stop() {
    if (player1Socket != INVALID_SOCKET) closesocket(player1Socket);
    if (player2Socket != INVALID_SOCKET) closesocket(player2Socket);
    if (listenSocket != INVALID_SOCKET) closesocket(listenSocket);
    WSACleanup();
    isRunning = false;
    std::cout << "Server network stopped." << std::endl;
}