

#ifndef POSSEMESTRALKA_GRID_H
#define POSSEMESTRALKA_GRID_H

#include "main.h"
#include "player.h"

typedef enum point{
    sea,
    miss,
    hit,
    ship
} POINT;

typedef enum direction {
    north,
    east,
    south,
    west
} DIR;

typedef enum ship {
    carrier = 5,
    battleship = 4,
    cruiser = 3,
    destroyer = 2,
    patrol = 1
} SHIP;



void initGrid(int * grid);

void writeGrid(int * grid1, int * grid2);

#endif //POSSEMESTRALKA_GRID_H
