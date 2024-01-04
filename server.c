#include <stdio.h>
#include <stdbool.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include "pos_sockets/passive_socket.h"
#include "pos_sockets/active_socket.h"
#include "grid.h"
#include "player.h"
#include "buffer.h"

//GENERATE_BUFFER(int,int)

typedef struct thread_data {
    PLAYER * player1;
    PLAYER * player2;
    int roundCount;

    bool gameEnd;

    pthread_mutex_t mutex;
    //pthread_cond_t serverPlacedShips;
    //pthread_cond_t clientPlacedShips;

    bool serverPlacedShips;
    bool clientPlacedShips;

    bool serverTurn;
    pthread_cond_t player1Turn;

    bool clientTurn;
    pthread_cond_t player2Turn;

    short port;
    ACTIVE_SOCKET* my_socket;
} THREAD_DATA;

void thread_data_init(struct thread_data* data, short port, ACTIVE_SOCKET* my_socket) {

    initPlayer(data->player1);
    initPlayer(data->player2);
    data->roundCount = 1;
    data->gameEnd = FALSE;

    data->serverPlacedShips = FALSE;
    data->clientPlacedShips = FALSE;


    pthread_mutex_init(&data->mutex, NULL);
    //pthread_cond_init(&data->serverPlacedShips, NULL);
    //pthread_cond_init(&data->clientPlacedShips, NULL);

    pthread_cond_init(&data->player1Turn, NULL);
    pthread_cond_init(&data->player2Turn, NULL);

    data->port = port;
    data->my_socket = my_socket;
}

void thread_data_destroy(struct thread_data* data) {

    pthread_mutex_destroy(&data->mutex);
    //pthread_cond_destroy(&data->serverPlacedShips);
    //pthread_cond_destroy(&data->clientPlacedShips);
    pthread_cond_destroy(&data->player1Turn);
    pthread_cond_destroy(&data->player2Turn);

    data->port = 0;
    data->my_socket = NULL;
}


void waitForClientToJoin(void* thread_data) {
    struct thread_data * data = thread_data;

    PASSIVE_SOCKET p_socket;
    passive_socket_init(&p_socket);
    passive_socket_start_listening(&p_socket, data->port);
    passive_socket_wait_for_client(&p_socket, data->my_socket);
    passive_socket_stop_listening(&p_socket);
    passive_socket_destroy(&p_socket);

    printf("Client connected!\n");
    active_socket_start_reading(data->my_socket);

}

void player1Turn(void * threadData, int x, int y){
    struct thread_data * data = threadData;

    //place a shot
    switch (getchar()) {
        case 'p':
            attack(data->player1, data->player2, (short)x, (short)y);
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
/*
void player2Turn(void * threadData, int x, int y){
    struct thread_data * data = threadData;
    bool end = false;
    //place a shot
    //TODO wrong char patch
    while(TRUE){
        switch (getchar()) {
            case 'p':
                attack(data->player2, data->player1, (short)x, (short)y);
                y = GRID_SIZE / 2;
                x = GRID_SIZE / 2;
                end = TRUE;
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
            case 's':  //right
                x++;
                if (x >= GRID_SIZE) x = GRID_SIZE - 1;
                printf("x: %d, y:%d\n", x, y);
                break;
            default:
                break;
        }
        if(end) break;
    }

}
*/

void fillBufferWithGrid(struct char_buffer* buf,int* grid){
    char  SGrid[GRID_SIZE*GRID_SIZE+1];
    int position;
    for (int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) {
        SGrid[i] = *grid++;
        printf("%d", SGrid[i]);
    }
    SGrid[100] = '\0';

    char_buffer_append(buf,SGrid,101);
}

void * process_server_data(void* thread_data){
    struct thread_data * data = thread_data;

    signed int x = GRID_SIZE / 2;
    signed int y = GRID_SIZE / 2;

    int direction = north;
    SHIP ships[] = {carrier, battleship, cruiser, destroyer, patrol};
    int placedShips = 0;
    int temp_grid[GRID_SIZE][GRID_SIZE];
    struct char_buffer buf;
    char_buffer_init(&buf);

    printf("#### PHASE ONE: PLACE YOUR SHIPS !!!\n");
    //loop for placing ships
    while (placedShips != NUM_OF_SHIP_CLASSES) {

        //copy grid, put ship on temperature grid and grid
        memcpy(temp_grid, data->player1->myGrid, GRID_SIZE * GRID_SIZE * sizeof(int));
        showPreview(temp_grid, ships[placedShips], direction, (short )x, (short )y);
        writeGrid(temp_grid[0], data->player2->myGrid[0]);

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
                if(!tryPutShipInGrid(data->player1->myGrid, ships[placedShips], direction, (short)x, (short)y)){
                    //tryPutShipInGrid(data->player2->myGrid, ships[placedShips], direction, (short)x, (short)y);
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

    pthread_mutex_lock(&data->mutex);
    data->serverPlacedShips = TRUE;
    pthread_mutex_unlock(&data->mutex);
    if (!data->clientPlacedShips)  printf("Waiting for Player 2 to place his ships!!\n");

    while(!data->clientPlacedShips){
        //wait for client to place ships
    }

    printf("#### PHASE TWO: BATTLE !!!\n");

    while(!data->gameEnd){

        printf("Round %d! Your turn ! Choose tile to attack !\n", data->roundCount);
        pthread_mutex_lock(&data->mutex);

        while(data->clientTurn){
            //wait for client (player 2) turn to end
            pthread_cond_wait(&data->player1Turn, &data->mutex);
        }

        data->serverTurn = TRUE;
        y = GRID_SIZE / 2;
        x = GRID_SIZE / 2;

        memcpy(temp_grid, data->player2->myGrid, GRID_SIZE * GRID_SIZE * sizeof(int));
        temp_grid[x][y] = target;
        writeGrid(data->player1->myGrid[0], temp_grid[0]);

        player1Turn(data, x, y);
        //TODO
        //send client grids of both players for display
        //fill buffer with data
        fillBufferWithGrid(&buf, data->player2->myGrid[0]);
        active_socket_write_data(data->my_socket, &buf);

        fillBufferWithGrid(&buf, data->player1->myGrid[0]);
        active_socket_write_data(data->my_socket, &buf);

        if (data->player2->lives <= 0){
            printf("Fleet of Player 2 has been destroyed! Game Over!\n");
            pthread_mutex_lock(&data->mutex);
            data->gameEnd = TRUE;
            pthread_mutex_unlock(&data->mutex);
            active_socket_write_end_message(data->my_socket);
        }

        data->serverTurn = FALSE;
        pthread_cond_signal(&data->player2Turn);

        pthread_mutex_unlock(&data->mutex);

    }




    return NULL;
}
bool tryDeserializePlacment(struct thread_data * data, struct char_buffer* buf) {
    char* position = strchr(buf->data, ';');
    if(position != NULL) {
        position = strchr(position+1, ';');
        if(position != NULL){
            //data protocol shipClass;x1;y1;x2;y2;x3;y3;x4;y4;x5;y5
            int shipClass,x1,y1,x2,y2,x3,y3,x4,y4,x5,y5;
            sscanf(buf->data,"%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;",&shipClass,&x1,&y1,&x2,&y2,&x3,&y3,&x4,&y4,&x5,&y5);
            pthread_mutex_lock(&data->mutex);
            switch (shipClass) {
                case carrier:
                    data->player2->myGrid[x1][y1] = ship;
                    data->player2->myGrid[x2][y2] = ship;
                    data->player2->myGrid[x3][y3] = ship;
                    data->player2->myGrid[x4][y4] = ship;
                    data->player2->myGrid[x5][y5] = ship;
                    break;
                case battleship:
                    data->player2->myGrid[x1][y1] = ship;
                    data->player2->myGrid[x2][y2] = ship;
                    data->player2->myGrid[x3][y3] = ship;
                    data->player2->myGrid[x4][y4] = ship;
                    break;
                case cruiser:
                    data->player2->myGrid[x1][y1] = ship;
                    data->player2->myGrid[x2][y2] = ship;
                    data->player2->myGrid[x3][y3] = ship;
                    break;
                case destroyer:
                    data->player2->myGrid[x1][y1] = ship;
                    data->player2->myGrid[x2][y2] = ship;
                    break;
                case patrol:
                    data->player2->myGrid[x1][y1] = ship;
                    break;
                default:
                    break;

            }
            pthread_mutex_unlock(&data->mutex);
            return true;
        }
    }
    return false;
}

bool tryDeserializeAttack(struct char_buffer* buf, int * x, int * y) {
    char* position = strchr(buf->data, ';');
    if(position != NULL) {
        position = strchr(position+1, ';');
        if(position != NULL){
            //int x,y;
            sscanf(buf->data, "%d;%d;", x, y);
            return true;
        }
    }
    return false;
}
void * process_client_data(void* thread_data){
    struct thread_data * data = thread_data;
    //TODO
    //start reading for placing ships
    CHAR_BUFFER r_buff;
    char_buffer_init(&r_buff);

    int placedShips = 0, x, y;
    while (placedShips != NUM_OF_SHIP_CLASSES) {

        active_socket_try_get_read_data(data->my_socket, &r_buff);
        if (active_socket_is_end_message(data->my_socket, &r_buff)){
            active_socket_stop_reading(data->my_socket);
        } else if (tryDeserializePlacment(data, &r_buff)){
            //return TRUE;
        } else{
            //wrong message from client
            active_socket_stop_reading(data->my_socket);
        }
        placedShips++;
    }

    //done placing ships
    pthread_mutex_lock(&data->mutex);
    data->clientPlacedShips = TRUE;
    pthread_mutex_unlock(&data->mutex);
    /*
    if (!data->serverPlacedShips)  printf("Waiting for Player 2 to place his ships!!\n");

    while(!data->serverPlacedShips){
        //wait for client to place ships
    }*/
    //start game
    while (!data->gameEnd){
        pthread_mutex_lock(&data->mutex);
        while(data->serverTurn){
            //wait for server (player 1) turn to end
            pthread_cond_wait(&data->player2Turn, &data->mutex);
        }

        //clients turn
        data->clientTurn = TRUE;
        if(tryDeserializeAttack(&r_buff,&x,&y)){
            //player2Turn(data, x, y);
            attack(data->player2, data->player1, x, y);
        } else{
            //something went wrong
        }
        data->clientTurn = FALSE;

        pthread_cond_signal(&data->player1Turn);
        pthread_mutex_unlock(&data->mutex);




    }

    //continue with game in rounds
    //game over





    return NULL;
}


int main() {
    pthread_t server;
    pthread_t client;


    struct active_socket my_socket;
    struct thread_data data;

    active_socket_init(&my_socket);
    thread_data_init(&data,19840, &my_socket);

    //wait for player 2 to join
    waitForClientToJoin(&data);

    //start with placing ships
    pthread_create(&server, NULL, process_server_data, &data);
    pthread_create(&client, NULL, process_client_data, &data);

    pthread_join(server, NULL);
    pthread_join(client, NULL);


    thread_data_destroy(&data);
    active_socket_destroy(&my_socket);

}