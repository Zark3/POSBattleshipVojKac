

#include "grid.h"



void initGrids(int * grid1, int * grid2){
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            grid1[j] = sea;

        }
    }
}

void writeGrid(int * grid1, int * grid2){
    char line = 'A';
    printf("______________________          ______________________\n");
    printf("_|0_1_2_3_4_5_6_7_8_9|          _|0_1_2_3_4_5_6_7_8_9|\n");
    for (int i = 0; i < 10; ++i) {
        printf("%c|",line);
        for (int j = 0; j < 10; ++j) {
            switch (grid1[j]) {
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
        for (int j = 0; j < 10; ++j) {
            switch (grid2[j]) {
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

        printf("\n");


    }

}