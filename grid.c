#include <stdio.h>
#include <string.h>
#include "grid.h"

void initGrid(int * grid){
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            *grid++ = sea;
        }
    }
}

int limitCoordinates(SHIP vessel, DIR direction, int * x, int * y){
    //if (*x < 0) {*x = 0;}
    //if (*x == GRID_SIZE) {*x = GRID_SIZE -1;}
    //if (*y < 0) {*y = 0;}
    //if (*y == GRID_SIZE) {*y = GRID_SIZE -1;}

    switch(direction) {
        case north:
            if ((*y - vessel + 1) == -1)  {*y = vessel - 1;}
            if (*x == GRID_SIZE) {*x = GRID_SIZE -1;}
            if (*x < 0) {*x = 0;}
            if (*y == GRID_SIZE) {*y = GRID_SIZE -1;}
            break;

        case east:
            if ((*x + vessel)  >= GRID_SIZE) {*x = GRID_SIZE - vessel;}
            if (*x < 0) {*x = 0;}
            if (*y < 0) {*y = 0;}
            if (*y == GRID_SIZE) {*y = GRID_SIZE -1;}
            break;

        case south:
            if ((*y + vessel) >= GRID_SIZE) {*y = GRID_SIZE - vessel;}
            if (*x < 0) {*x = 0;}
            if (*x == GRID_SIZE) {*x = GRID_SIZE -1;}
            if (*y < 0) {*y = 0;}
            break;

        case west:
            if ((*x - vessel + 1) == -1) {*x = vessel - 1;}
            if (*y < 0) {*y = 0;}
            if (*x == GRID_SIZE) {*x = GRID_SIZE -1;}
            if (*y == GRID_SIZE) {*y = GRID_SIZE -1;}
            break;
    }

    return 0;
}

void writeGrid(int * grid1, int * grid2){
    char line = 'A';
    printf("______________________          ______________________\n");
    printf("_|0_1_2_3_4_5_6_7_8_9|          _|0_1_2_3_4_5_6_7_8_9|\n");
    for (int i = 0; i < GRID_SIZE; ++i) {
        printf("%c|",line);
        for (int j = 0; j < GRID_SIZE; ++j) {
            switch (*grid1++) {
            //switch (grid1[i][j]) {
                case sea:
                    printf("~");
                    break;
                case miss:
                    printf("o");
                    break;
                case hit:
                    printf("x");
                    break;
                case ship:
                    printf("s");
                    break;
            }
            printf("|");
        }
        printf("          ");
        printf("%c|",line++);
        for (int j = 0; j < GRID_SIZE; ++j) {
            switch (*grid2++) {
                case ship:

                case sea:
                    printf("~");
                    break;
                case miss:
                    printf("o");
                    break;
                case hit:
                    printf("x");
                    break;

                case target:
                    printf("@");
                    break;
            }
            printf("|");
        }

        printf("\n");
    }
}



int tryPutShipInGrid(int grid[][GRID_SIZE], SHIP vessel, DIR dir, short x, short y) {
    printf("Trying to put ship %d of player, in direcion %d, from x=%d, y=%d\n", vessel, dir, x, y);

    //check boundaries
    switch (dir) {
        case north:
            if (y - vessel < 0){
                printf("Error placing your ship. Ship was out of boundaries! Try again.\n");
                return 1;
            }
            break;
        case east:
            if (x + vessel > GRID_SIZE){
                printf("Error placing your ship. Ship was out of boundaries! Try again.\n");
                return 1;
            }
            break;
        case south:
            if (y + vessel > GRID_SIZE){
                printf("Error placing your ship. Ship was out of boundaries! Try again\n");
                return 1;
            }
            break;
        case west:
            if (x - vessel < 0){
                printf("Error placing your ship. Ship was out of boundaries! Try again\n");
                return 1;
            }
            break;
    }

    //check already placed ships
    short temp_x = x;
    short temp_y = y;
    for (int i = 0; i < vessel-1; ++i) {
        switch (dir) {
            case north:
                if(grid[--temp_y][temp_x] == ship) {
                    printf("Error placing your ship. Another ship is in the way on x:%d, y:%d ! Try again\n", temp_x, temp_y);
                    return 1;
                }
                continue;
            case east:
                if(grid[temp_y][++temp_x] == ship) {
                    printf("Error placing your ship. Another ship is in the way on x:%d, y:%d ! Try again\n", temp_x, temp_y);
                    return 1;
                }
                continue;
            case south:
                if(grid[++temp_y][temp_x] == ship) {
                    printf("Error placing your ship. Another ship is in the way on x:%d, y:%d ! Try again\n", temp_x, temp_y);
                    return 1;
                }
                continue;
            case west:
                if(grid[temp_y][--temp_x] == ship) {
                    printf("Error placing your ship. Another ship is in the way on x:%d, y:%d ! Try again\n", temp_x, temp_y);
                    return 1;
                }
                continue;
        }
    }

    //print info about ship placement
    switch (vessel) {
        case carrier:
            printf("Carrier(5 squares) was placed on square %c%d\n", y+65, x);
            break;
        case battleship:
            printf("Battleship(4 squares) was placed on square %c%d\n", y+65, x);
            break;
        case cruiser:
            printf("Cruiser(3 squares) was placed on square %c%d\n", y+65, x);
            break;
        case destroyer:
            printf("Destroyer(2 squares) was placed on square %c%d\n", y+65, x);
            break;
        case patrol:
            printf("Patrol boat(1 square) was placed on square %c%d\n", y+65, x);
            break;
    }

    //place ship to grid
    grid[y][x] = ship;
    for (int i = 0; i < vessel-1; ++i) {
        switch (dir) {
            case north:
                grid[--y][x] = ship;
                break;
            case east:
                grid[y][++x] = ship;
                break;
            case south:
                grid[++y][x] = ship;
                break;
            case west:
                grid[y][--x] = ship;
                break;
        }
    }
    return 0;
}


void showPreview(int grid[][GRID_SIZE], SHIP vessel, DIR dir, short x, short y) {
    grid[y][x] = ship;
    for (int i = 0; i < vessel-1; ++i) {
        switch (dir) {
            case north:
                grid[--y][x] = ship;
                break;
            case east:
                grid[y][++x] = ship;
                break;
            case south:
                grid[++y][x] = ship;
                break;
            case west:
                grid[y][--x] = ship;
                break;
        }
    }
}

void getShipData(int x, int y, DIR direction, int data[10]) {

    for (int i = 0; i < 10; ++i) {
        switch (direction) {
            case north:
                data[i] = x;
                data[++i] = y--;
                break;
            case east:
                data[i] = x++;
                data[++i] = y;
                break;
            case south:
                data[i] = x;
                data[++i] = y++;
                break;
            case west:
                data[i] = x--;
                data[++i] = y;
                break;
        }
    }
}
char * getDirectionString(DIR direction){
    switch (direction) {
        case 0: return "north";
        case 1: return "east";
        case 2: return "south";
        case 3: return "west";
    }
}

char * getShipString(SHIP vessel){
    switch (vessel) {
        case carrier: return " aircraft carrier";
        case battleship: return "battleship";
        case cruiser: return "cruiser";
        case destroyer: return "destroyer";
        case patrol: return "patrol ship";
    }
}