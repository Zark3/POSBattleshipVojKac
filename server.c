#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
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
#include <ncurses.h>

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

    bool noShot;

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

    data->serverTurn = TRUE;
    data->noShot = false;


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
    printf("Waiting for client to join\r\n");
    passive_socket_wait_for_client(&p_socket, data->my_socket);
    passive_socket_stop_listening(&p_socket);
    passive_socket_destroy(&p_socket);

    //printf("Client connected!\r\n");
    //active_socket_start_reading(data->my_socket);

}


void fillBufferWithGrid(char* buf,int* grid){
    //char  SGrid[GRID_SIZE*GRID_SIZE+1];
    //int position;
    for (int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) {
        buf[i] = *grid++;
        //printf("%d", buf[i]);
    }
    buf[100] = '\0';
    printf("\n");
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

    printf("#### PHASE ONE: PLACE YOUR SHIPS !!!\r\n");
    //loop for placing ships

    while (placedShips != NUM_OF_SHIP_CLASSES) {

        //copy grid, put ship on temperature grid and grid
        memcpy(temp_grid, data->player1->myGrid, GRID_SIZE * GRID_SIZE * sizeof(int));
        showPreview(temp_grid, ships[placedShips], direction, (short )x, (short )y);
        writeGrid(temp_grid[0], data->player2->myGrid[0]);

        printf("Place the %s!\r\n", getShipString(ships[placedShips]));
        if(data->noShot){
            printf("Couldn't placed %s! Try again!\r\n", getShipString(ships[placedShips]));
        }

        switch (getchar()) {
            case 'r':
                //printf("r===== %d\n", y - ships[placedShips] + 1);
                switch((direction + 1) % 4){
                    case north:
                        //printf("r===== %d\n", y - ships[placedShips] + 1);
                        if ((y - ships[placedShips] + 1) <= -1)  {++direction; direction %= 4;}
                        break;

                    case east:
                        //printf("r===== %d\n", x + ships[placedShips] - 1);
                        if ((x + ships[placedShips] - 1) < GRID_SIZE) {++direction; direction %= 4;}
                        break;

                    case south:
                        //printf("r===== %d\n", y + ships[placedShips] - 1);
                        if ((y + ships[placedShips] - 1) < GRID_SIZE) {++direction; direction %= 4;}
                        break;

                    case west:
                        //printf("r===== %d\n", x - ships[placedShips] + 1);
                        if ((x - ships[placedShips] + 1) <= -1) {++direction; direction %= 4;}
                        break;
                }
                //printf("direction: %s\r\n", getDirectionString(direction));
                break;
            case 'p':  //enter ship
                if(!tryPutShipInGrid(data->player1->myGrid, ships[placedShips], direction, (short)x, (short)y)){
                    //tryPutShipInGrid(data->player2->myGrid, ships[placedShips], direction, (short)x, (short)y);
                    printf("%s. placed!\r\n", getShipString(ships[placedShips]));
                    data->noShot = FALSE;
                    placedShips++;
                    direction = north;
                    x = GRID_SIZE / 2;
                    y = GRID_SIZE / 2;
                } else {
                    data->noShot = TRUE;
                }
                break;
            case 'w':  //up
                y--;
                //printf("x: %d, y:%d\r\n", x, y);
                break;
            case 's':  //down
                y++;
                //printf("x: %d, y:%d\r\n", x, y);
                break;
            case 'a':  //left
                x--;
                //printf("x: %d, y:%d\r\n", x, y);
                break;
            case 'd':  //right
                x++;
                //printf("x: %d, y:%d\r\n", x, y);
                break;
            case 'b':
                active_socket_stop_reading(data->my_socket);
                break;
        }
        limitCoordinates(ships[placedShips], direction, &x, &y);
        //clear screen
        //getchar();
        system("clear");
        printf("#### PHASE ONE: PLACE YOUR SHIPS !!!\r\n");
    }

    pthread_mutex_lock(&data->mutex);

    data->serverPlacedShips = TRUE;

    pthread_mutex_unlock(&data->mutex);

    writeGrid(data->player1->myGrid[0], data->player2->myGrid[0]);

    if (!data->clientPlacedShips) {
        printf("Waiting for Player 2 to place his ships!!\r\n");
    }

    while(!data->clientPlacedShips){
        //wait for client to place ships

    }

    system("clear");
    printf("#### PHASE TWO: BATTLE !!!\r\n");

    while(!data->gameEnd) {


        pthread_mutex_lock(&data->mutex);

        while (!data->serverTurn) {
            //wait for client (player 2) turn to end
            pthread_cond_wait(&data->player1Turn, &data->mutex);
        }

        data->serverTurn = TRUE;
        y = GRID_SIZE / 2;
        x = GRID_SIZE / 2;


        //place a shot
        while (TRUE) {
            system("clear");
            printf("#### PHASE TWO: BATTLE !!!\r\n");
            memcpy(temp_grid, data->player2->myGrid, GRID_SIZE * GRID_SIZE * sizeof(int));
            temp_grid[x][y] = target;
            writeGrid(data->player1->myGrid[0], temp_grid[0]);
            if(data->noShot){
                printf("We have already fired on that spot. Try another square!\r\n");
            }

            printf("Round %d! Your turn ! Choose tile to attack !\r\n", data->roundCount);

            switch (getchar()) {
                case 'p':
                    if(attack(data->player1, data->player2, (short) x, (short) y) == 0){
                        data->noShot = true;
                        continue;
                    }

                    data->roundCount++;
                    data->noShot = false;
                    goto end;
                    break;
                case 'a':  //up
                    y--;
                    if (y < 0) y = 0;
                    //printf("x: %d, y:%d\r\n", x, y);
                    break;
                case 'd':  //down
                    y++;
                    if (y >= GRID_SIZE) y = GRID_SIZE - 1;
                    //printf("x: %d, y:%d\r\n", x, y);
                    break;
                case 'w':  //left
                    x--;
                    if (x < 0) x = 0;
                    //printf("x: %d, y:%d\r\n", x, y);
                    break;
                case
                    's':  //right
                    x++;
                    if (x >= GRID_SIZE) x = GRID_SIZE - 1;
                    //printf("x: %d, y:%d\r\n", x, y);
                    break;
            }
        }
        end:
        //TODO
        //send client grids of both players for display
        //fill buffer with data


        if (data->player2->lives <= 0) {
            system("clear");
            printf("#### PHASE TWO: BATTLE !!!\r\n");
            writeGrid(data->player1->myGrid[0], data->player2->myGrid[0]);
            printf("Enemy fleet has been destroyed! Good job! Game Over!\r\n");
            data->serverTurn = false;
            data->gameEnd = TRUE;
            pthread_cond_signal(&data->player2Turn);
            pthread_mutex_unlock(&data->mutex);
            break;

            //active_socket_write_end_message(data->my_socket);
        } else {
            system("clear");
            printf("#### PHASE TWO: BATTLE !!!\r\n");
            writeGrid(data->player1->myGrid[0], data->player2->myGrid[0]);
            lastAttackInfo((short) x, (short) y, data->player2);
            y = GRID_SIZE / 2;
            x = GRID_SIZE / 2;
            printf("Waiting for player 2 to play his round!\r\n");



            data->serverTurn = FALSE;
            pthread_cond_signal(&data->player2Turn);
            pthread_cond_wait(&data->player1Turn, &data->mutex);

            pthread_mutex_unlock(&data->mutex);


        }
    }


    //printf("Thread process_server_data ending!\r\n");
    return NULL;
}
bool tryDeserializePlacment(struct thread_data * data, char* buf) {
    int shipClass, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
    int iter = 0;


    shipClass = buf[0];
    //shipClass = atoi(&buf[0]);

    x1 = buf[2];
    y1 = buf[4];
    x2 = buf[6];
    y2 = buf[8];
    x3 = buf[10];
    y3 = buf[12];
    x4 = buf[14];
    y4 = buf[16];
    x5 = buf[18];
    y5 = buf[20];

    //printf("Deserialized data: %s\r\n", buf);
    //printf("Ship: %d, x1:%d, y1:%d, x2:%d, y2:%d, x3:%d, y3:%d, x4:%d, y4:%d, x5:%d, y5:%d\r\n", shipClass, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5);
    pthread_mutex_lock(&data->mutex);
    switch (shipClass) {
        case carrier:
            data->player2->myGrid[y1][x1] = ship;
            data->player2->myGrid[y2][x2] = ship;
            data->player2->myGrid[y3][x3] = ship;
            data->player2->myGrid[y4][x4] = ship;
            data->player2->myGrid[y5][x5] = ship;
            break;
        case battleship:
            data->player2->myGrid[y1][x1] = ship;
            data->player2->myGrid[y2][x2] = ship;
            data->player2->myGrid[y3][x3] = ship;
            data->player2->myGrid[y4][x4] = ship;
            break;
        case cruiser:
            data->player2->myGrid[y1][x1] = ship;
            data->player2->myGrid[y2][x2] = ship;
            data->player2->myGrid[y3][x3] = ship;
            break;
        case destroyer:
            data->player2->myGrid[y1][x1] = ship;
            data->player2->myGrid[y2][x2] = ship;
            break;
        case patrol:
            data->player2->myGrid[y1][x1] = ship;
            break;
        default:
            break;

    }
    pthread_mutex_unlock(&data->mutex);
    return true;

}

bool tryDeserializeAttack(char* buf, int * x, int * y) {

    *x = buf[0];
    *y = buf[2];

    return true;
}
void * process_client_data(void* thread_data){
    struct thread_data * data = thread_data;
    //TODO
    //start reading for placing ships
    char buffer[150];
    CHAR_BUFFER r_buff;
    char_buffer_init(&r_buff);

    int placedShips = 0, x, y;
    while (placedShips != NUM_OF_SHIP_CLASSES) {
        //printf("Trying to read data\r\n");
        read(data->my_socket->socket_descriptor, buffer, 100);

        if (tryDeserializePlacment(data, buffer)){
            //printf("PLACEMENT DATA RECIEVED\r\n");
            placedShips++;
        } else{
            //wrong message from client
            //active_socket_stop_reading(data->my_socket);
            printf("Wrong data from client!\n");
            break;
        }
    }

    //done placing ships
    pthread_mutex_lock(&data->mutex);
    data->clientPlacedShips = TRUE;
    pthread_mutex_unlock(&data->mutex);

    while(!data->serverPlacedShips){
        //wait for player 1 to place ships
    }
    strcpy(buffer, "start");
    write(data->my_socket->socket_descriptor, buffer, 10);



    //start game
    while (!data->gameEnd){
        pthread_mutex_lock(&data->mutex);
        while(data->serverTurn){
            //wait for server (player 1) turn to end
            pthread_cond_wait(&data->player2Turn, &data->mutex);
        }



        //clients turn
        data->serverTurn = FALSE;

        strcpy(buffer, "turn");
        write(data->my_socket->socket_descriptor, buffer, 10);
        //printf("Wrote %s to client\r\n", buffer);

        //printf("Waiting for clients message to update\r\n");
        read(data->my_socket->socket_descriptor, buffer, 10);

        while(strcmp(buffer, "update") != 0){
            read(data->my_socket->socket_descriptor, buffer, 10);
        }
        //printf("Updating client\r\n");
        //sleep(1);
        fillBufferWithGrid(buffer, data->player2->myGrid[0]);
        write(data->my_socket->socket_descriptor, buffer, 150);
        //active_socket_write_data(data->my_socket, buffer);
        //sleep(1);
        fillBufferWithGrid(buffer, data->player1->myGrid[0]);
        write(data->my_socket->socket_descriptor, buffer, 150);
        //active_socket_write_data(data->my_socket, &buf);

        if (data->player2->lives <= 0){
            //printf("Enemy fleet has been destroyed! Good job! Game Over!\r\n");
            pthread_mutex_unlock(&data->mutex);
            break;
            //active_socket_write_end_message(data->my_socket);
        }

        read(data->my_socket->socket_descriptor, buffer, 10);

        if(tryDeserializeAttack(buffer,&x,&y)){
            //player2Turn(data, x, y);
            attack(data->player2, data->player1, (short)x, (short)y);
        } else{
            //something went wrong
        }

        //game over
        if (data->player1->lives <= 0){
            printf("Your fleet has been destroyed! You failed! Game Over!\r\n");
            data->serverTurn = TRUE;
            data->gameEnd = TRUE;
            strcpy(buffer, "turn");
            write(data->my_socket->socket_descriptor, buffer, 10);
            //printf("Wrote %s to client\r\n", buffer);

            //printf("Waiting for clients message to update\r\n");
            read(data->my_socket->socket_descriptor, buffer, 10);

            while(strcmp(buffer, "update") != 0){
                read(data->my_socket->socket_descriptor, buffer, 10);
            }
            //printf("Updating client\r\n");
            //sleep(1);
            fillBufferWithGrid(buffer, data->player2->myGrid[0]);
            write(data->my_socket->socket_descriptor, buffer, 150);
            //active_socket_write_data(data->my_socket, buffer);
            //sleep(1);
            fillBufferWithGrid(buffer, data->player1->myGrid[0]);
            write(data->my_socket->socket_descriptor, buffer, 150);




            //active_socket_write_end_message(data->my_socket);
        }
        data->serverTurn = TRUE;

        pthread_cond_signal(&data->player1Turn);
        pthread_mutex_unlock(&data->mutex);




    }






    //printf("Thread process_client_data ending!\r\n");
    return NULL;
}


int main() {
    initscr();
    noecho();
    system("clear");

    pthread_t server;
    pthread_t client;

    PLAYER player1;
    PLAYER player2;


    struct active_socket my_socket;
    struct thread_data data;

    data.player1 = &player1;
    data.player2 = &player2;

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
    endwin();
}