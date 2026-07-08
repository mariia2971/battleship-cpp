#include <iostream>
#include "Server.h"

int main() {
    // Enable console output encoding support
    setlocale(LC_ALL, "Russian");
    std::cout << "=== BATTLESHIP SERVER STARTUP ===" << std::endl;

    Server gameServer;

    // Start the server on port 60000
    if (gameServer.Start(60000)) {
        gameServer.WaitForPlayers(); // Wait for players to connect
        gameServer.GameLoop();       // Run the game loop
    }

    gameServer.Stop(); // Close sockets on exit
    std::cout << "=== SERVER SHUTDOWN COMPLETE ===" << std::endl;
    return 0;
}