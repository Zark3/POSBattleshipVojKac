#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include "pos_sockets/char_buffer.h"
#include "grid.h"
#include "client.h"

enum winner {
    clientWinner,
    serverWinner,
    none
};

struct thread_data{
    PLAYER clientPlayer;
    PLAYER serverPlayer;

    bool gameEnd;
    bool gameUpdate;
    int roundCount;

    int sck_tcp;

    pthread_cond_t update;
    pthread_cond_t play;
    pthread_mutex_t mutex;

    struct char_buffer * buffer;
};

int createConnection(char * address, short port) {
    struct sockaddr_in serv_addr;
    struct hostent* server;

    server = gethostbyname(address);
    if (server == NULL) {
        perror("Retrieving server information failed");
        return -1;
    }

    //set server address to zero
    memset((char*)&serv_addr,0, sizeof (serv_addr));

    //set IPv4 = AF_INET
    serv_addr.sin_family = AF_INET;

    //set port
    serv_addr.sin_port = htons(port);

    //set address
    //copy server address from server to serv_addr
    memcpy((char*)&serv_addr.sin_addr.s_addr, (char*)server->h_addr, server->h_length);

    int sck_tcp;
    //create socket, default setting, IPv4
    sck_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if  (sck_tcp  < 0) {
        perror("Creation of socket failed");
        return -1;
    }

    //connect to server
    if (connect(sck_tcp, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {
        perror("Connection to server failed");
        fprintf(stderr, "Address: %s, Port: %d\n", address, port);
        return -1;
    }

    return sck_tcp;
}

int sendData(int sck_dscr,struct char_buffer * buffer) {
    if (write(sck_dscr, buffer->data, buffer->size) < 0) {
        perror("Error writing to socket");
        return -2;
    }
    return 0;
}

int receiveData(int sck_dscr, struct char_buffer * buffer) {
    char recData[GRID_SIZE * GRID_SIZE + 1];

    if(read(sck_dscr, recData, 101) < 0)
    {
        perror("Error reading from socket");
        return -3;
    }

    char_buffer_clear(buffer);
    char_buffer_append(buffer, recData, GRID_SIZE*GRID_SIZE + 1);

    return 0;
}

void thread_data_init(struct thread_data * data){

    initPlayer(&data->clientPlayer);
    initPlayer(&data->serverPlayer);

    data->gameEnd = false;
    data->gameUpdate = false;
    data->roundCount = 1;

    pthread_mutex_init(&data->mutex, NULL);
    pthread_cond_init(&data->update, NULL);
    pthread_cond_init(&data->play, NULL);

    char_buffer_init(data->buffer);

    //server info
    struct hostent* server;

    //server address
    struct sockaddr_in serv_addr;

    //socket descriptor
    //data->sck_tcp = createConnection(SERV_ADDR, PORT, server, &serv_addr);
    if (data->sck_tcp < 0) {
        perror("Connecting to server failed");
    }
}

void thread_data_destroy(struct  thread_data * data){

    pthread_cond_destroy(&data->play);
    pthread_cond_destroy(&data->update);
    pthread_mutex_destroy(&data->mutex);

    char_buffer_destroy(data->buffer);

    close(data->sck_tcp);
}



void serializeAttack(int x, int y, struct char_buffer * buffer){
     char_buffer_clear(buffer);

     char serializedAttack[] = {(char)x,';', (char)y, ';'};

     char_buffer_append(buffer, serializedAttack, 4);
}


void serializeShipPlacement(SHIP shipClass, int coordinates[], struct char_buffer * buffer) {
    char_buffer_clear(buffer);

    char serializedPlacement[22];

    serializedPlacement[0] = shipClass;
    serializedPlacement[1] = ';';
    serializedPlacement[2] = coordinates[0];
    serializedPlacement[3] = ';';
    serializedPlacement[4] = coordinates[1];
    serializedPlacement[5] = ';';
    serializedPlacement[6] = coordinates[2];
    serializedPlacement[7] = ';';
    serializedPlacement[8] = coordinates[3];
    serializedPlacement[9] = ';';
    serializedPlacement[10] = coordinates[4];
    serializedPlacement[11] = ';';
    serializedPlacement[12] = coordinates[5];
    serializedPlacement[13] = ';';
    serializedPlacement[14] = coordinates[6];
    serializedPlacement[15] = ';';
    serializedPlacement[16] = coordinates[7];
    serializedPlacement[17] = ';';
    serializedPlacement[18] = coordinates[8];
    serializedPlacement[19] = ';';
    serializedPlacement[20] = coordinates[9];
    serializedPlacement[21] = ';';

    char_buffer_append(buffer, serializedPlacement, 22);
}

void deserializeGrid(struct char_buffer * buffer, int *grid) {
    for(int i = 0; i < GRID_SIZE * GRID_SIZE; i++)
    {
        *grid++ = buffer->data[i];
    }
}

int receiveGameGrid(int sck_descr, int grid[][GRID_SIZE]) {

    return 0;
}
void * updateGameData(void * gameData) {
    struct thread_data * data = gameData;

    pthread_mutex_lock(&data->mutex);

    while(data->gameUpdate == false) {
        //wait for player to place target and send data, then receive upgraded grid from server
        pthread_cond_wait(&data->play, &data->mutex);
    }

    //get clients grid
    receiveData(data->sck_tcp, data->buffer);
    deserializeGrid(data->buffer, data->clientPlayer.myGrid[0]);

    //get server grid
    receiveData(data->sck_tcp, data->buffer);
    deserializeGrid(data->buffer, data->serverPlayer.myGrid[0]);

    data->gameUpdate = false;
    pthread_cond_signal(&data->play);
    pthread_mutex_unlock(&data->mutex);


    return NULL;
}

int determineWinner(int *gridClient, int *gridServer){
    int shipsClient = 0;
    int shipsServer = 0;
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        if(*gridServer == ship){shipsServer++;}
        if(*gridClient++ == ship){shipsClient++;}
    }

    if (shipsServer == 0) {
        printf("Fleet of enemy has been destroyed! Game Over!\n");
        return clientWinner;
    }

    if (shipsClient == 0) {
        printf("Your fleet has been destroyed! Game Over!!\n");
        return serverWinner;
    }

    else return 0;
}

void * play(void * gameData){
    struct thread_data * data = gameData;

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
        memcpy(temp_grid, data->clientPlayer.myGrid, GRID_SIZE * GRID_SIZE * sizeof(int));
        showPreview(temp_grid, ships[placedShips], direction, (short )x, (short )y);
        writeGrid(temp_grid[0], data->clientPlayer.myGrid[0]);

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
                if(!tryPutShipInGrid(data->clientPlayer.myGrid, ships[placedShips], direction, (short)x, (short)y)){
                    printf("%s. placed!\n", getShipString(ships[placedShips]));
                    placedShips++;

                    //process ship placement and send do server
                    int shipCoordinates[10];
                    getShipData(x, y, direction, shipCoordinates);
                    serializeShipPlacement(ships[placedShips], shipCoordinates, data->buffer);
                    sendData(data->sck_tcp, data->buffer);

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
    if(data->gameUpdate) printf("Wait for another Player to place his ships!\n");

    printf("#### PHASE TWO: BATTLE !!!\n");

    while(determineWinner(data->clientPlayer.myGrid[0], data->serverPlayer.myGrid[0])) {
        //TODO wait for your turn

        printf("Round %d! Your turn ! Choose tile to attack !\n", data->roundCount++);
        pthread_mutex_lock(&data->mutex);
        while(data->gameUpdate){
            //for grid update
            pthread_cond_wait(&data->update, &data->mutex);
        }

        y = GRID_SIZE / 2;
        x = GRID_SIZE / 2;

        //copy opposites players grid for target placement
        memcpy(temp_grid, data->serverPlayer.myGrid, GRID_SIZE * GRID_SIZE * sizeof(int));
        temp_grid[x][y] = target;
        writeGrid(data->clientPlayer.myGrid[0], temp_grid[0]);

        //place a shot
        switch (getchar()) {
            case 'p':

                //process target placement and send to server
                serializeAttack(x,y, data->buffer);
                sendData(data->sck_tcp, data->buffer);

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

    data->gameUpdate = true;
    pthread_cond_signal(&data->update);

    pthread_mutex_unlock(&data->mutex);
    return NULL;
}

int main() {

    //socket descriptor
    int sck_tcp;
    sck_tcp = createConnection(SERV_ADDR, PORT);
    if (sck_tcp < 0) {
        perror("Connecting to server failed");
        return 2;
    }

    //thread data
    struct thread_data data;
    CHAR_BUFFER buf;
    data.buffer = &buf;
    thread_data_init(&data);

    pthread_t game;
    pthread_t update;

    pthread_create(&game, NULL, play, &data);
    pthread_create(&update, NULL, updateGameData, &data);

    pthread_join(game, NULL);
    pthread_join(update, NULL);


    thread_data_destroy(&data);

    return 0;
}
