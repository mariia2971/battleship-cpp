#pragma once
#include <cstdint>
#include <string>

// Handles everything about the game state: fields, ships, hit checking, drawing
class GameLogic {
private:
    int myField[10][10];      // 0=empty, 1=ship, 2=hit, 3=miss (on my field)
    int enemyField[10][10];   // 0=unknown, 2=hit, 3=miss (enemy field)
    bool gameOver;
    bool myTurn;              // true = it's this player's turn

    bool IsShipSunk(int x, int y) const;   // check if the ship at (x,y) is fully destroyed
    void MarkAroundShip(int x, int y);     // mark surrounding cells as misses

public:
    GameLogic();

    void PlaceShips();               // automatic random placement
    void DrawFields() const;         // print both fields to console

    // Process an enemy's shot. Returns true if it hit (or destroyed). Sets outResult and outGameOver.
    bool ProcessEnemyShot(int x, int y, HitResult& outResult, bool& outGameOver);

    // Apply the result of our own shot (update enemy field)
    void ApplyShotResult(int x, int y, HitResult result, bool gameOverFlag);

    // Getters/setters
    bool IsGameOver() const { return gameOver; }
    bool IsMyTurn() const { return myTurn; }
    void SetMyTurn(bool turn) { myTurn = turn; }

    int playerNumber;
    std::string playerName;
    std::string enemyName;
};