#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "Constants.h"

bool findMatches(int board[SIZE][SIZE], bool match[SIZE][SIZE]);
void removeMatches(int board[SIZE][SIZE], bool match[SIZE][SIZE], int& score);
void applyGravity(int board[SIZE][SIZE]);
void addNewPieces(int board[SIZE][SIZE]);
void processAllMatches(int board[SIZE][SIZE], int& score);
bool trySwap(int board[SIZE][SIZE], int x1, int y1, int x2, int y2, int& moves, int& score);

#endif // GAMELOGIC_H