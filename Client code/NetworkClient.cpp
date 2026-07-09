#include "NetworkClient.h"
#include <iostream>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

NetworkClient::NetworkClient(const std::string& addr, int port)
    : serverAddr(addr), port(port), sock(INVALID_SOCKET), connected(false) {
}

NetworkClient::~NetworkClient() {
    Disconnect();
}

bool NetworkClient::InitializeWinsock() {
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        std::cerr << "WSAStartup failed: " << res << std::endl;
        return false;
    }
    return true;
}

bool NetworkClient::ResolveAndConnect() {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        return false;
    }

    struct addrinfo hints, * result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::string portStr = std::to_string(port);
    int iResult = getaddrinfo(serverAddr.c_str(), portStr.c_str(), &hints, &result);
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed: " << iResult << std::endl;
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        iResult = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == 0) break;
    }
    freeaddrinfo(result);

    if (iResult != 0) {
        std::cerr << "Connect failed." << std::endl;
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }
    return true;
}

bool NetworkClient::Connect() {
    if (!InitializeWinsock()) return false;
    if (!ResolveAndConnect()) {
        WSACleanup();
        return false;
    }
    connected = true;
    return true;
}

void NetworkClient::Disconnect() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    WSACleanup();
    connected = false;
}

bool NetworkClient::SendPacket(const void* data, int size) {
    if (!connected) return false;
    int sent = 0;
    const char* ptr = (const char*)data;
    while (sent < size) {
        int res = send(sock, ptr + sent, size - sent, 0);
        if (res == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            connected = false;
            return false;
        }
        sent += res;
    }
    return true;
}

bool NetworkClient::ReceivePacket(void* buffer, int size) {
    if (!connected) return false;
    int received = 0;
    char* ptr = (char*)buffer;
    while (received < size) {
        int res = recv(sock, ptr + received, size - received, 0);
        if (res <= 0) {
            if (res == 0) std::cout << "Server closed connection." << std::endl;
            else std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
            connected = false;
            return false;
        }
        received += res;
    }
    return true;
}

bool NetworkClient::SendHello(const std::string& name) {
    MsgClientHello hello;
    hello.header.type = PacketType::ClientHello;
    hello.header.size = sizeof(MsgClientHello);
    memset(hello.playerName, 0, sizeof(hello.playerName));
    strncpy_s(hello.playerName, name.c_str(), _TRUNCATE);
    return SendPacket(&hello, sizeof(hello));
}

bool NetworkClient::ReceiveMatchFound(MsgMatchFound& out) {
    if (!ReceivePacket(&out, sizeof(out))) return false;
    if (out.header.type != PacketType::MatchFound) {
        std::cerr << "Expected MatchFound, got " << (int)out.header.type << std::endl;
        return false;
    }
    return true;
}

bool NetworkClient::SendFire(int x, int y) {
    MsgFire fire;
    fire.header.type = PacketType::Fire;
    fire.header.size = sizeof(MsgFire);
    fire.x = static_cast<uint8_t>(x);
    fire.y = static_cast<uint8_t>(y);
    return SendPacket(&fire, sizeof(fire));
}

bool NetworkClient::SendFireResult(const MsgFireResult& result) {
    return SendPacket(&result, sizeof(result));
}