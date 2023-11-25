#include "main.h"


void userInput(DIR * direction1, SHIP ship1, int *x, int *y) {
    int result = 1, help;

    char all[BUFFSIZE], first = 0, second = 0;
    //char first = 0;
    //char second = 0;

    switch (ship1) {
        case carrier:
            printf("Time for placing your Carrier\n");
            break;
        case battleship:
            printf("Time for placing your Battleship\n");
            break;
        case cruiser:
            printf("Time for placing your Cruiser\n");
            break;
        case destroyer:
            printf("Time for placing your Destroyer\n");
            break;
        case patrol:
            printf("Time for placing your Patrol boat\n");
            break;
        default:
            break;
    }
    while (result != 0) {
        printf("Write coordinates of square you want to place your ship. Like A5, F7,...\n");
        while (help != '\n') {
            //clearing stdin buffer
            help = getchar();
        }
        fgets(all, 4,stdin);
        for (int i = 0; i < BUFFSIZE; ++i) {
            if (all[i] == '\n'){
                all[i] = '\0';
            }
        }
        printf("You wrote %s\n", all);
        first = all[0];
        printf("First is %c\n", first);
        printf("First is %d\n", first);
        second = all[1];
        printf("Second is %c\n", second);
        printf("Second is %d\n", second);

        if(((first >= 65 && first <= 74) || (first >= 97 && first <= 106)) && (second >= 48 && second <= 57)) {
            if(first >= 65 && first <= 74) {
                *y = first - 65;
            } else {
                *y = first - 97;
            }
            *x = atoi(&second);
            result = 0;
        }
    }

    result = 1;
    memset(all,'\0', BUFFSIZE);
    help = '\0';

    printf("Write direction for ships placement. up, down, left, right\n");
    while (result != 0) {
        while (help != '\n') {
            //clearing stdin buffer
            help = getchar();
        }
        fgets(all, BUFFSIZE/2,stdin);
        for (int i = 0; i < BUFFSIZE; ++i) {
            if (all[i] == '\n'){
                all[i] = '\0';
            }
        }
        printf("%s was read\n", all);
        if(strcmp(all, "up") == 0){
            *direction1 = north;
            result = 0;
        } else if(strcmp(all, "down") == 0) {
            *direction1 = south;
            result = 0;
        } else if(strcmp(all, "left") == 0) {
            *direction1 = west;
            result = 0;
        } else if(strcmp(all, "right") == 0) {
            *direction1 = east;
            result = 0;
        } else {
            printf("Wrong direction of ship! Try again!\n");
            result = 1;
        }
    }

    memset(all,'\0', BUFFSIZE);
    help = '\0';
    while ((help = getchar()) != '\n') {
        //clearing stdin buffer

    }

}

int main() {
    PLAYER player1;
    PLAYER player2;

    initPlayer(&player1);
    initPlayer(&player2);

    printf("Welcome to Battleship!\nPrepare your fleet for fight by placing your ships on battleground\n");


    SHIP ship1 = carrier;

    while(player1.placedShips < 5) {
        writeGrid(player1.myGrid[0], player2.myGrid[0]);
        DIR * direction;
        int x, y;
        userInput(direction, ship1, &x, &y);

        if(!tryPutShipInGrid(&player1, ship1, *direction, x, y)){
            printf("Placement of ship was a success\n");
            ship1--;
            continue;
        }
    }

    /*
    while(player1.lives < 0 || player2.lives < 0){
        writeGrid(player1.myGrid[0], player2.myGrid[0]);

    }
    */


    //tryPutShipInGrid(&player2, battleship, south, 0, 0);
    //tryPutShipInGrid(&player1, cruiser, east, 8, 7);
    writeGrid(player1.myGrid[0], player2.myGrid[0]);
/*
    attack(&player1, &player2, 0 ,1);

    writeGrid(player1.myGrid[0], player2.myGrid[0]);

    attack(&player2, &player1, 6 ,2);

    writeGrid(player1.myGrid[0], player2.myGrid[0]);
*/
    return 0;
}


