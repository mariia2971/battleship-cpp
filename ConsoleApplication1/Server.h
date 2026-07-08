#pragma once
#include <WinSock2.h>
#include <iostream>

/**
 * @brief Class for managing network connections of the game server.
 */
class Server {
private:
    SOCKET listenSocket;          // Socket that listens on port 60000
    SOCKET player1Socket;         // First player's socket
    SOCKET player2Socket;         // Second player's socket
    bool isRunning;               // Server running flag

public:
    Server();                     // Constructor
    ~Server();                    // Destructor (will automatically close network on exit)

    bool Start(int port);         // Starts the network and begins listening on the port
    void WaitForPlayers();        // Waits for two players to connect
    void GameLoop();              // Forwards packets from one player to another
    void Stop();                  // Completely shuts down the server
};
