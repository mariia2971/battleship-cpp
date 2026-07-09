#pragma once
#include <WinSock2.h>
#include <string>
#include "NetworkProtocol.h"

class NetworkClient {
private:
    SOCKET sock;
    std::string serverAddr;
    int port;
    bool connected;

    bool InitializeWinsock();
    bool ResolveAndConnect();

public:
    NetworkClient(const std::string& addr, int port);
    ~NetworkClient();

    bool Connect();
    void Disconnect();

    // Send/receive with full data handling
    bool SendPacket(const void* data, int size);
    bool ReceivePacket(void* buffer, int size);

    // High-level protocol functions
    bool SendHello(const std::string& name);
    bool ReceiveMatchFound(MsgMatchFound& out);
    bool SendFire(int x, int y);
    bool SendFireResult(const MsgFireResult& result);

    SOCKET GetSocket() const { return sock; }
    bool IsConnected() const { return connected; }
};