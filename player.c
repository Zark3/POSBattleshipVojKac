#include "player.h"
#include <stdio.h>

void initPlayer(PLAYER * player){
    player->lives = 17;
    player->placedShips = 0;
    initGrid(player->myGrid[0]);
}

int attack(PLAYER *att, PLAYER *def, short x, short y) {
    if (def->myGrid[x][y] == ship){
        def->myGrid[x][y] = hit;
        //printf("Nice shot! We have hit the enemy on square %c%d\n", x+65, y);
        def->lives--;
        return 1;
    }else if (def->myGrid[x][y] == sea) {
        def->myGrid[x][y] = miss;
        //printf("Our shell fell into water! No hit on square %c%d\n", x+65, y);
        return 2;
    } else if (def->myGrid[x][y] == hit){
        //printf("We have already fired on that spot. Try another square then %c%d!\n", x+65, y);
        return 0;
    }
    return 4;
}



