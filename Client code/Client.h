#pragma once
#include <WinSock2.h>
#include <string>
#include "NetworkProtocol.h"

class Client {
private:
    SOCKET sock;
    std::string serverAddress;
    int serverPort;
    std::string playerName;
    int playerNumber;          // 1 или 2
    std::string enemyName;
    bool isMyTurn;             // чей сейчас ход
    bool gameOver;

    // Игровые поля (10x10)
    // 0 – пусто, 1 – корабль, 2 – подбит, 3 – мимо
    int myField[10][10];
    int enemyField[10][10];

    bool InitializeWinsock();
    bool ConnectToServer();
    bool SendPacket(const void* data, int size);
    bool ReceivePacket(void* buffer, int size);
    bool SendHello();
    bool ReceiveMatchFound();
    void PlaceShips();          // расстановка кораблей (автоматическая или ручная)
    void DrawFields();
    void MakeTurn();
    void ProcessFire(const MsgFire& fire);
    void ProcessFireResult(const MsgFireResult& result);

public:
    Client(const std::string& addr, int port, const std::string& name);
    ~Client();
    void Run();                 // главный цикл
};