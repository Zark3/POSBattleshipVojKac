

#include "grid.h"



void initGrid(int * grid){
    for (int i = 0; i < GRIDSIZE; ++i) {
        for (int j = 0; j < GRIDSIZE; ++j) {
            *grid++ = sea;

        }
    }
}

void writeGrid(int * grid1, int * grid2){
    char line = 'A';
    printf("______________________          ______________________\n");
    printf("_|0_1_2_3_4_5_6_7_8_9|          _|0_1_2_3_4_5_6_7_8_9|\n");
    for (int i = 0; i < GRIDSIZE; ++i) {
        printf("%c|",line);
        for (int j = 0; j < GRIDSIZE; ++j) {
            switch (*grid1++) {
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
        for (int j = 0; j < GRIDSIZE; ++j) {
            switch (*grid2++) {
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
                    printf("~");
                    break;
            }
            printf("|");
        }

        printf("\n");


    }

}

