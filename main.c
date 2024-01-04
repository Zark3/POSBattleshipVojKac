#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "grid.h"
#include "player.h"

int main() {
    //initscr();
    //keypad(stdscr, TRUE);
    PLAYER player;
    initPlayer(&player);

    PLAYER player2;
    initPlayer(&player2);

    signed int x = GRID_SIZE / 2;
    signed int y = GRID_SIZE / 2;

    int direction = north;
    SHIP ships[] = {carrier, battleship, cruiser, destroyer, patrol};
    int placedShips = 0;
    int temp_grid[GRID_SIZE][GRID_SIZE];

    printf("#### PHASE ONE: PLACE YOUR SHIPS !!!\n");

    //loop for placing ships
    while (placedShips != NUM_OF_SHIP_CLASSES) {

        //copy grid, put ship on temperature grid and grid
        memcpy(temp_grid, player.myGrid, GRID_SIZE * GRID_SIZE * sizeof(int));
        showPreview(temp_grid, ships[placedShips], direction, (short )x, (short )y);
        writeGrid(temp_grid[0], player2.myGrid[0]);

        printf("Place the %s!\n", getShipString(ships[placedShips]));

        switch (getchar()) {
            case 'r':
                printf("r===== %d\n", y - ships[placedShips] + 1);
                switch((direction + 1) % 4){
                    case north:
                        printf("r===== %d\n", y - ships[placedShips] + 1);
                        //TODO nejde limitovanie otacanaia na sever
                        if ((y - ships[placedShips] + 1) <= -1)  {++direction; direction %= 4;}
                        break;

                    case east:
                        printf("r===== %d\n", x + ships[placedShips] - 1);
                        if ((x + ships[placedShips] - 1) < GRID_SIZE) {++direction; direction %= 4;}
                        break;

                    case south:
                        printf("r===== %d\n", y + ships[placedShips] - 1);
                        if ((y + ships[placedShips] - 1) < GRID_SIZE) {++direction; direction %= 4;}
                        break;

                    case west:
                        printf("r===== %d\n", x - ships[placedShips] + 1);
                        if ((x - ships[placedShips] + 1) <= -1) {++direction; direction %= 4;}
                        break;
                }
                printf("direction: %s\n", getDirectionString(direction));
                break;
            case 'p':  //enter ship
                if(!tryPutShipInGrid(player.myGrid, ships[placedShips], direction, (short)x, (short)y)){
                    !tryPutShipInGrid(player2.myGrid, ships[placedShips], direction, (short)x, (short)y);
                    printf("%s. placed!\n", getShipString(ships[placedShips]));
                    placedShips++;
                    direction = north;
                    x = GRID_SIZE / 2;
                    y = GRID_SIZE / 2;
                }
                break;
            case 'w':  //up
                y--;
                printf("x: %d, y:%d\n", x, y);
                break;
            case 's':  //down
                y++;
                printf("x: %d, y:%d\n", x, y);
                break;
            case 'a':  //left
                x--;
                printf("x: %d, y:%d\n", x, y);
                break;
            case 'd':  //right
                x++;
                printf("x: %d, y:%d\n", x, y);
                break;
        }

        limitCoordinates(ships[placedShips], direction, &x, &y);


        //clear screen
        system("clear");
    }

    bool GameEnd = false;
    y = GRID_SIZE / 2;
    x = GRID_SIZE / 2;
    while(1) {
        //TODO update grids

        //TODO check if your a winner

        //TODO show if your on a turn

        //TODO wait for your turn

        //copy opposites players grid for target placement
        memcpy(temp_grid, player2.myGrid, GRID_SIZE * GRID_SIZE * sizeof(int));
        temp_grid[x][y] = target;
        writeGrid(player.myGrid[0], temp_grid[0]);

        //place a shot
        switch (getchar()) {
            case 'p':
                attack(&player, &player2, (short)x, (short)y);
                y = GRID_SIZE / 2;
                x = GRID_SIZE / 2;
                break;
            case 'a':  //up
                y--;
                if (y < 0) y = 0;
                printf("x: %d, y:%d\n", x, y);
                break;
            case 'd':  //down
                y++;
                if (y >= GRID_SIZE) y = GRID_SIZE - 1;
                printf("x: %d, y:%d\n", x, y);
                break;
            case 'w':  //left
                x--;
                if (x < 0) x = 0;
                printf("x: %d, y:%d\n", x, y);
                break;
            case
            's':  //right
                x++;
                if (x >= GRID_SIZE) x = GRID_SIZE - 1;
                printf("x: %d, y:%d\n", x, y);
                break;
        }
    }
    endwin();
    return 0;
}
