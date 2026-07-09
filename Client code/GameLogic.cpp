#include "GameLogic.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>

GameLogic::GameLogic() : gameOver(false), myTurn(false), playerNumber(0) {
    memset(myField, 0, sizeof(myField));
    memset(enemyField, 0, sizeof(enemyField));
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void GameLogic::PlaceShips() {
    int ships[] = { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };
    int idx = 0;
    while (idx < 10) {
        int size = ships[idx];
        bool placed = false;
        for (int attempt = 0; attempt < 100 && !placed; ++attempt) {
            int x = std::rand() % 10;
            int y = std::rand() % 10;
            bool horizontal = (std::rand() % 2 == 0);
            if (horizontal && x + size > 10) continue;
            if (!horizontal && y + size > 10) continue;

            bool ok = true;
            for (int i = -1; i <= size; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    int cx = horizontal ? x + i : x + j;
                    int cy = horizontal ? y + j : y + i;
                    if (cx < 0 || cx >= 10 || cy < 0 || cy >= 10) continue;
                    if (myField[cy][cx] != 0) { ok = false; break; }
                }
                if (!ok) break;
            }
            if (!ok) continue;

            for (int i = 0; i < size; ++i) {
                int cx = horizontal ? x + i : x;
                int cy = horizontal ? y : y + i;
                myField[cy][cx] = 1;
            }
            placed = true;
        }
        if (placed) idx++;
        else {
            memset(myField, 0, sizeof(myField));
            idx = 0;
        }
    }
    std::cout << "Ships placed automatically.\n";
}

void GameLogic::DrawFields() const {
    system("cls");
    std::cout << "=== YOUR FIELD (Player " << (int)playerNumber << ") ===\n";
    std::cout << "  0 1 2 3 4 5 6 7 8 9\n";
    for (int y = 0; y < 10; ++y) {
        std::cout << y << " ";
        for (int x = 0; x < 10; ++x) {
            char ch;
            switch (myField[y][x]) {
            case 0: ch = '.'; break;
            case 1: ch = '#'; break;
            case 2: ch = 'X'; break;
            case 3: ch = 'o'; break;
            default: ch = '?';
            }
            std::cout << ch << ' ';
        }
        std::cout << std::endl;
    }

    std::cout << "\n=== ENEMY FIELD ===\n";
    std::cout << "  0 1 2 3 4 5 6 7 8 9\n";
    for (int y = 0; y < 10; ++y) {
        std::cout << y << " ";
        for (int x = 0; x < 10; ++x) {
            char ch;
            switch (enemyField[y][x]) {
            case 0: ch = '.'; break;
            case 2: ch = 'X'; break;
            case 3: ch = 'o'; break;
            default: ch = '?';
            }
            std::cout << ch << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

bool GameLogic::IsShipSunk(int x, int y) const {
    bool visited[10][10] = { false };
    int queueX[100], queueY[100];
    int head = 0, tail = 0;
    queueX[tail] = x; queueY[tail] = y; tail++;
    visited[y][x] = true;
    bool allHit = true;

    while (head < tail) {
        int cx = queueX[head], cy = queueY[head]; head++;
        if (myField[cy][cx] == 1) allHit = false;
        int dx[] = { 1, -1, 0, 0 };
        int dy[] = { 0, 0, 1, -1 };
        for (int i = 0; i < 4; ++i) {
            int nx = cx + dx[i], ny = cy + dy[i];
            if (nx >= 0nx < 10 && ny >= 0 && ny < 10 && !visited[ny][nx]) {
                if (myField[ny][nx] == 1 || myField[ny][nx] == 2) {
                    visited[ny][nx] = true;
                    queueX[tail] = nx; queueY[tail] = ny; tail++;
                }
            }
        }
    }
    return allHit;
}

void GameLogic::MarkAroundShip(int x, int y) {
    bool visited[10][10] = { false };
    int queueX[100], queueY[100];
    int head = 0, tail = 0;
    queueX[tail] = x; queueY[tail] = y; tail++;
    visited[y][x] = true;

    while (head < tail) {
        int cx = queueX[head], cy = queueY[head]; head++;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = cx + dx, ny = cy + dy;
                if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10) {
                    if (myField[ny][nx] == 0) myField[ny][nx] = 3;
                }
            }
        }
        int dirs[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
        for (int i = 0; i < 4; ++i) {
            int nx = cx + dirs[i][0], ny = cy + dirs[i][1];
            if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10 && !visited[ny][nx]) {
                if (myField[ny][nx] == 2) {
                    visited[ny][nx] = true;
                    queueX[tail] = nx; queueY[tail] = ny; tail++;
                }
            }
        }
    }
}

bool GameLogic::ProcessEnemyShot(int x, int y, HitResult& outResult, bool& outGameOver) {
    bool hit = (myField[y][x] == 1);
    if (hit) {
        myField[y][x] = 2;
        bool sunk = IsShipSunk(x, y);
        outResult = sunk ? HitResult::Destroyed : HitResult::Hit;
        if (sunk) {
            MarkAroundShip(x, y);
        }
    }
    else {
        if (myField[y][x] == 0) myField[y][x] = 3;
        outResult = HitResult::Miss;
    }

    bool allSunk = true;
    for (int yy = 0; yy < 10 && allSunk; ++yy)
        for (int xx = 0; xx < 10; ++xx)
            if (myField[yy][xx] == 1) { allSunk = false; break; }
    outGameOver = allSunk;
    return hit;
}

void GameLogic::ApplyShotResult(int x, int y, HitResult result, bool gameOverFlag) {
    if (result == HitResult::Hit || result == HitResult::Destroyed)
        enemyField[y][x] = 2;
    else
        enemyField[y][x] = 3;
    gameOver = gameOverFlag;
}