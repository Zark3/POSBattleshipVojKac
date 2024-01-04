#ifndef POSSEMESTRALKA_PLAYER_H
#define POSSEMESTRALKA_PLAYER_H

#include "grid.h"

typedef struct player {
    short lives;
    int myGrid[10][10];
    short placedShips;
} PLAYER;

void initPlayer(PLAYER * player);
int attack(PLAYER * att, PLAYER * def, short x, short y);


#endif //POSSEMESTRALKA_PLAYER_H
