#include <stdio.h>
#include "grid.h"

int main() {

    int player1Grid[10][10] = {0};
    int player2Grid[10][10] = {0};

    initGrids( player1Grid[0], player2Grid[0]);
    writeGrid(player1Grid[0], player2Grid[0]);
    return 0;
}
