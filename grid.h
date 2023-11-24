
#ifndef POSSEMESTRALKA_GRID_H
#define POSSEMESTRALKA_GRID_H

#include <stdio.h>

typedef enum point{
    sea,
    miss,
    hit,
    ship
} POINT;



void initGrids(int * grid1, int * grid2);
void writeGrid(int * grid1, int* grid2);

#endif //POSSEMESTRALKA_GRID_H
