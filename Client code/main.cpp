#include <iostream>
#include <string>
#include "NetworkClient.h"
#include "GameLogic.h"

int main() {
    setlocale(LC_ALL, "Russian");  // optional, for Cyrillic output

    std::string address, name;
    int port;

    std::cout << "Enter server address (e.g., 127.0.0.1 or 0.tcp.ngrok.io): ";
    std::cin >> address;
    std::cout << "Enter port: ";
    std::cin >> port;
    std::cout << "Enter your name: ";
    std::cin >> name;

    NetworkClient netClient(address, port);
    if (!netClient.Connect()) {
        std::cerr << "Could not connect to server." << std::endl;
        return 1;
    }
    std::cout << "Connected to server.\n";

    // Send hello
    if (!netClient.SendHello(name)) {
        std::cerr << "Failed to send hello." << std::endl;
        netClient.Disconnect();
        return 1;
    }
    std::cout << "Hello sent. Waiting for MatchFound...\n";

    // Wait for MatchFound from server
    MsgMatchFound match;
    if (!netClient.ReceiveMatchFound(match)) {
        std::cerr << "Failed to receive MatchFound." << std::endl;
        netClient.Disconnect();
        return 1;
    }

    // Initialize game logic
    GameLogic game;
    game.playerNumber = match.playerNumber;
    game.enemyName = match.enemyName;
    game.playerName = name;
    game.SetMyTurn(match.playerNumber == 1); // Player 1 starts

    std::cout << "You are Player " << game.playerNumber << ". Enemy: " << game.enemyName << std::endl;
    if (game.IsMyTurn()) std::cout << "You start!" << std::endl;
    else std::cout << "Wait for enemy's move." << std::endl;

    game.PlaceShips();

    bool gameOver = false;
    bool connectionAlive = true;

    // Main game loop
    while (!gameOver && connectionAlive) {
        game.DrawFields();

        if (game.IsMyTurn()) {
            std::cout << "Your turn. Enter coordinates (x y): ";
            int x, y;
            std::cin >> x >> y;
            if (x < 0 || x > 9 || y < 0 || y > 9) {
                std::cout << "Invalid coordinates. Try again.\n";
                continue;
            }
            if (!netClient.SendFire(x, y)) {
                connectionAlive = false;
                break;
            }
            game.SetMyTurn(false);
            std::cout << "Shot sent. Waiting for result...\n";
        }
        else {
            std::cout << "Waiting for enemy's move...\n";
        }

        // Wait for any incoming packet
        PacketHeader header;
        if (!netClient.ReceivePacket(&header, sizeof(header))) {
            connectionAlive = false;
            break;
        }
        int bodySize = header.size - sizeof(header);
        char buffer[256];
        if (bodySize > 0) {
            if (!netClient.ReceivePacket(buffer, bodySize)) {
                connectionAlive = false;
                break;
            }
        }

        // Process packet by type
        switch (header.type) {
        case PacketType::Fire: {
            MsgFire fire;
            memcpy(&fire, &header, sizeof(header));
            if (bodySize > 0) memcpy((char*)&fire + sizeof(header), buffer, bodySize);
            HitResult result;
            bool gameOverFlag;
            bool hit = game.ProcessEnemyShot(fire.x, fire.y, result, gameOverFlag);
            MsgFireResult fireResult;
            fireResult.header.type = PacketType::FireResult;
            fireResult.header.size = sizeof(MsgFireResult);
            fireResult.x = fire.x;
            fireResult.y = fire.y;
            fireResult.result = result;
            fireResult.isGameOver = gameOverFlag;
            if (!netClient.SendFireResult(fireResult)) {
                connectionAlive = false;
                break;
            }
            if (gameOverFlag) {
                gameOver = true;
                std::cout << "Game Over! You " << (hit ? "lost!" : "won!") << std::endl;
            }
            break;
        }
        case PacketType::FireResult: {
            MsgFireResult result;
            memcpy(&result, &header, sizeof(header));
            if (bodySize > 0) memcpy((char*)&result + sizeof(header), buffer, bodySize);
            game.ApplyShotResult(result.x, result.y, result.result, result.isGameOver);
            if (result.isGameOver) {
                gameOver = true;
                std::cout << "Game Over! You " << (result.result == HitResult::Destroyed ? "won!" : "lost!") << std::endl;
            }
            else {
                game.SetMyTurn(true); // our turn again
            }
            break;
        }
        case PacketType::PlayerLeft: {
            std::cout << "Enemy left the game. You win!" << std::endl;
            gameOver = true;
            break;
        }
        default:
            std::cout << "Unknown packet type: " << (int)header.type << std::endl;
            break;
        }
    }

    std::cout << "Game ended. Press Enter to exit..." << std::endl;
    std::cin.get(); std::cin.get();
    netClient.Disconnect();
    return 0;
}