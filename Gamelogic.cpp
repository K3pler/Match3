#include "GameLogic.h"
#include <cstdlib>
#include <cmath>

bool findMatches(int board[SIZE][SIZE], bool match[SIZE][SIZE]) {
    bool found = false;

    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
            match[y][x] = false;

    // Горизонтальные комбинации
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE - 2; x++) {
            int kind = board[y][x];
            if (kind == -1) continue;

            int len = 1;
            while (x + len < SIZE && board[y][x + len] == kind) len++;

            if (len >= 3) {
                found = true;
                for (int k = 0; k < len; k++) match[y][x + k] = true;
            }
            x += len - 1;
        }
    }

    // Вертикальные комбинации
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE - 2; y++) {
            int kind = board[y][x];
            if (kind == -1) continue;

            int len = 1;
            while (y + len < SIZE && board[y + len][x] == kind) len++;

            if (len >= 3) {
                found = true;
                for (int k = 0; k < len; k++) match[y + k][x] = true;
            }
            y += len - 1;
        }
    }
    return found;
}

void removeMatches(int board[SIZE][SIZE], bool match[SIZE][SIZE], int& score) {
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if (match[y][x]) {
                board[y][x] = -1;
                score += 11;
            }
        }
    }
}

void applyGravity(int board[SIZE][SIZE]) {
    for (int x = 0; x < SIZE; x++) {
        for (int y = SIZE - 1; y >= 0; y--) {
            if (board[y][x] == -1) {
                for (int up = y - 1; up >= 0; up--) {
                    if (board[up][x] != -1) {
                        board[y][x] = board[up][x];
                        board[up][x] = -1;
                        break;
                    }
                }
            }
        }
    }
}

void addNewPieces(int board[SIZE][SIZE]) {
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if (board[y][x] == -1) {
                board[y][x] = rand() % 4;
            }
        }
    }
}

void processAllMatches(int board[SIZE][SIZE], int& score) {
    bool match[SIZE][SIZE];
    while (findMatches(board, match)) {
        removeMatches(board, match, score);
        applyGravity(board);
        addNewPieces(board);
    }
}

bool trySwap(int board[SIZE][SIZE], int x1, int y1, int x2, int y2, int& moves, int& score) {
    if (abs(x1 - x2) + abs(y1 - y2) != 1) return false;

    int temp = board[y1][x1];
    board[y1][x1] = board[y2][x2];
    board[y2][x2] = temp;

    int oldScore = score;
    processAllMatches(board, score);

    if (score == oldScore) {
        temp = board[y1][x1];
        board[y1][x1] = board[y2][x2];
        board[y2][x2] = temp;
        return false;
    }

    moves++;
    return true;
}