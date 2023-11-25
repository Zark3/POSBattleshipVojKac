


#ifndef POSSEMESTRALKA_PLAYER_H
#define POSSEMESTRALKA_PLAYER_H

#include "main.h"
#include "grid.h"

typedef struct player {
    short lives;
    int myGrid[GRIDSIZE][GRIDSIZE];
    short placedShips;
} PLAYER;

void initPlayer(PLAYER * player);
int tryPutShipInGrid(PLAYER * player, enum ship, enum direction, short x, short y);
int attack(PLAYER * att, PLAYER * def, short x, short y);

#endif //POSSEMESTRALKA_PLAYER_H
