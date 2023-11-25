
#include "player.h"


void initPlayer(PLAYER * player){
    player->lives = 17;
    player->placedShips = 0;
    initGrid(player->myGrid[0]);

}

int tryPutShipInGrid(PLAYER * player, SHIP ship, DIR dir, short x, short y) {
    printf("Trying to put ship %d of player, in direcion %d, from x=%d, y=%d\n", ship, dir, x, y);

    switch (dir) {
        case north:
            if (y - ship < 0){
                printf("Error placing your ship. Ship was out of boundaries! Try again.\n");
                return 1;
            }
            break;
        case east:
            if (x + ship > 9){
                printf("Error placing your ship. Ship was out of boundaries! Try again.\n");
                return 1;
            }
            break;
        case south:
            if (y + ship > 9){
                printf("Error placing your ship. Ship was out of boundaries! Try again");
                return 1;
            }
            break;
        case west:
            if (x - ship < 0){
                printf("Error placing your ship. Ship was out of boundaries! Try again");
                return 1;
            }
            break;
    }

    switch (ship) {
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

    player->myGrid[y][x] = 3;
    for (int i = 0; i < ship-1; ++i) {
        switch (dir) {
            case north:
                player->myGrid[--y][x] = 3;
                break;
            case east:
                player->myGrid[y][++x] = 3;
                break;
            case south:
                player->myGrid[++y][x] = 3;
                break;
            case west:
                player->myGrid[y][--x] = 3;
                break;
        }
    }

    return 0;
}

int attack(PLAYER *att, PLAYER *def, short x, short y) {
    if (def->myGrid[y][x] == ship){
        def->myGrid[y][x] = hit;
        printf("Nice shot! We have hit the enemy on square %c%d\n", y+65, x);
        def->lives--;
        return 1;
    }else if (def->myGrid[y][x] == sea) {
        def->myGrid[y][x] = miss;
        printf("Our shell fell into water! No hit on square %c%d\n", y+65, x);
        return 2;
    } else if (def->myGrid[y][x] == hit){
        printf("We have already fired on that spot. Try another square then %c%d!\n", y+65, x);
        return 0;
    }
    return 4;
}
