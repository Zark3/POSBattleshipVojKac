#ifndef POSSEMESTRALKA_GRID_H
#define POSSEMESTRALKA_GRID_H

#include "player.h"

#define GRID_SIZE 10
#define NUM_OF_SHIP_CLASSES 5
#define TOTAL_SHIP_COLUMNS 15

typedef enum point{
    sea,
    miss,
    hit,
    ship,
    target
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

int limitCoordinates(SHIP vessel, DIR direction, int * x, int *  y);

int tryPutShipInGrid(int grid[][GRID_SIZE], SHIP ship, DIR direction, short x, short y);

void showPreview(int  players_grid[][GRID_SIZE], SHIP ship, DIR dir, short x, short y);

void getShipData(int x, int y, DIR direction, int data[11]);

char * getDirectionString(DIR direction);

char * getShipString(SHIP vessel);

#endif //POSSEMESTRALKA_GRID_H
