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
#include <ncurses.h>

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
        fprintf(stderr, "Address: %s, Port: %d\r\n", address, port);
        return -1;
    }

    return sck_tcp;
}

int sendData(int sck_dscr,struct char_buffer * buffer) {
    //printf("Sending data: %s to %d socket\r\n", buffer->data, sck_dscr);
    if (write(sck_dscr, buffer->data, buffer->size) < 0) {
        perror("Error writing to socket");
        return -2;
    }
    return 0;
}

int receiveData(int sck_dscr, struct char_buffer * buffer) {
    char recData[150];

    //printf("Ready for data read\r\n");

    if(read(sck_dscr, recData, 150) < 0)
    {
        perror("Error reading from socket");
        return -3;
    }

    char_buffer_clear(buffer);
    char_buffer_append(buffer, recData, 150);
    //printf("Read data: %s\r\n",buffer->data);

    return 0;
}

void thread_data_init(struct thread_data * data){

    initPlayer(&data->clientPlayer);
    initPlayer(&data->serverPlayer);

    data->gameEnd = false;
    data->gameUpdate = true;




    pthread_mutex_init(&data->mutex, NULL);
    pthread_cond_init(&data->update, NULL);
    pthread_cond_init(&data->play, NULL);

    char_buffer_init(data->buffer);

    //server info
    struct hostent* server;

    //server address
    struct sockaddr_in serv_addr;

    //socket descriptor
    for (int i = 0; i < 4; ++i) {
        data->sck_tcp = createConnection(SERV_ADDR, PORT);
        if (data->sck_tcp < 0) {
            perror("Connecting to server failed\r\n");
            perror("Trying again in 5s\r\n");
            sleep(5);
            continue;
        }
        break;
    }

}

void thread_data_destroy(struct  thread_data * data){

    pthread_cond_destroy(&data->play);
    pthread_cond_destroy(&data->update);
    pthread_mutex_destroy(&data->mutex);

    char_buffer_destroy(data->buffer);

    close(data->sck_tcp);
}

int sendEndMessage(int sck_dscr,struct char_buffer * buffer) {
    char_buffer_clear(buffer);
    char_buffer_append(buffer, ":end", 4);
    if (write(sck_dscr, buffer->data, buffer->size) < 0) {
        perror("Error writing to socket");
        return -2;
    }
    return 0;
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
    //printf("Deserialized grid: ");
    for(int i = 0; i < GRID_SIZE * GRID_SIZE; i++)
    {
        *grid++ = buffer->data[i];
        //printf("%d", buffer->data[i]);
    }
    printf("\r\n");
}

int determineWinner(int *gridClient, int *gridServer){
    int shipsClient = 0;
    int shipsServer = 0;
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        if(gridServer[i] == ship){shipsServer++;}
        //printf("%d ",gridServer[i]);
        if(gridClient[i] == ship){shipsClient++;}
        //printf("%d\n",gridServer[i]);
    }

    //printf("Client hp is %d\r\n", shipsClient);
    //printf("Server hp is %d\r\n", shipsServer);


    if (shipsServer == 0) {
        printf("Fleet of enemy has been destroyed! Game Over!\r\n");

        return clientWinner;
    }

    if (shipsClient == 0) {
        printf("Your fleet has been destroyed! Game Over!!\r\n");
        return serverWinner;
    }

    else return none;
}

void * updateGameData(void * gameData) {
    struct thread_data * data = gameData;

    while (true){
        pthread_mutex_lock(&data->mutex);

        //while(data->gameUpdate) {
        //wait for player to place target and send data, then receive upgraded grid from server
        pthread_cond_wait(&data->update, &data->mutex);
        //}
        //printf("Starting to update grids\r\n");
        //get clients grid
        char_buffer_clear(data->buffer);
        char_buffer_append(data->buffer, "update", 7);
        sendData(data->sck_tcp, data->buffer);

        receiveData(data->sck_tcp, data->buffer);
        deserializeGrid(data->buffer, data->clientPlayer.myGrid[0]);

        //get server grid
        receiveData(data->sck_tcp, data->buffer);
        deserializeGrid(data->buffer, data->serverPlayer.myGrid[0]);

        data->gameUpdate = true;
        pthread_cond_signal(&data->play);
        pthread_mutex_unlock(&data->mutex);

        if(determineWinner(data->clientPlayer.myGrid[0], data->serverPlayer.myGrid[0]) != none){
        //if(data->gameEnd == true){
            break;
        }

    }
    return NULL;
}

void * play(void * gameData){
    struct thread_data * data = gameData;

    signed int x = GRID_SIZE / 2;
    signed int y = GRID_SIZE / 2;

    int direction = north;
    SHIP ships[] = {carrier, battleship, cruiser, destroyer, patrol};
    int placedShips = 0;
    int temp_grid[GRID_SIZE][GRID_SIZE];


    printf("#### PHASE ONE: PLACE YOUR SHIPS !!!\r\n");

    //loop for placing ships
    while (placedShips != NUM_OF_SHIP_CLASSES) {

        //copy grid, put ship on temperature grid and grid
        memcpy(temp_grid, data->clientPlayer.myGrid, GRID_SIZE * GRID_SIZE * sizeof(int));
        showPreview(temp_grid, ships[placedShips], direction, (short )x, (short )y);
        writeGrid(temp_grid[0], data->clientPlayer.myGrid[0]);


        printf("Place the %s!\r\n", getShipString(ships[placedShips]));

        switch (getchar()) {
            case 'r':
               //printf("r===== %d\r\n", y - ships[placedShips] + 1);
                switch((direction + 1) % 4){
                    case north:
                        //printf("r===== %d\r\n", y - ships[placedShips] + 1);
                        if ((y - ships[placedShips] + 1) <= -1)  {++direction; direction %= 4;}
                        break;

                    case east:
                        //printf("r===== %d\r\n", x + ships[placedShips] - 1);
                        if ((x + ships[placedShips] - 1) < GRID_SIZE) {++direction; direction %= 4;}
                        break;

                    case south:
                        //printf("r===== %d\r\n", y + ships[placedShips] - 1);
                        if ((y + ships[placedShips] - 1) < GRID_SIZE) {++direction; direction %= 4;}
                        break;

                    case west:
                        //printf("r===== %d\r\n", x - ships[placedShips] + 1);
                        if ((x - ships[placedShips] + 1) <= -1) {++direction; direction %= 4;}
                        break;
                }
                //printf("direction: %s\r\n", getDirectionString(direction));
                break;
            case 'p':  //enter ship
                if(!tryPutShipInGrid(data->clientPlayer.myGrid, ships[placedShips], direction, (short)x, (short)y)){
                    //printf("%s. placed!\r\n", getShipString(ships[placedShips]));


                    //process ship placement and send do server
                    int shipCoordinates[10];
                    getShipData(x, y, direction, shipCoordinates);
                    serializeShipPlacement(ships[placedShips], shipCoordinates, data->buffer);
                    sendData(data->sck_tcp, data->buffer);

                    direction = north;
                    x = GRID_SIZE / 2;
                    y = GRID_SIZE / 2;
                    placedShips++;
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
        }

        limitCoordinates(ships[placedShips], direction, &x, &y);


        //clear screen
        //getchar();
        system("clear");
        printf("#### PHASE ONE: PLACE YOUR SHIPS !!!\r\n");
    }

    system("clear");
    //send done message and wait for server response
    writeGrid(data->clientPlayer.myGrid[0], data->serverPlayer.myGrid[0]);
    printf("Wait for another Player to place his ships!\r\n");
    read(data->sck_tcp, data->buffer->data, 10);
    //printf("Received %s from server\r\n", data->buffer->data);

    while(strcmp(data->buffer->data, "start") != 0){
        read(data->sck_tcp, data->buffer->data, 10);
    }

    system("clear");
    printf("#### PHASE TWO: BATTLE !!!\r\n");
    writeGrid(data->clientPlayer.myGrid[0], data->serverPlayer.myGrid[0]);
    printf("Waiting for turn!\r\n");
    receiveData(data->sck_tcp, data->buffer);

    while(strcmp(data->buffer->data, "turn") !=0 ){
        receiveData(data->sck_tcp, data->buffer);
    }


    pthread_mutex_lock(&data->mutex);
    pthread_cond_signal(&data->update);
    //printf("Signaled update thread\r\n");

    pthread_cond_wait(&data->play,&data->mutex);
    pthread_mutex_unlock(&data->mutex);
    /*
    data->gameUpdate = false;
    receiveData(data->sck_tcp, data->buffer);
    deserializeGrid(data->buffer, data->clientPlayer.myGrid[0]);

    //get server grid
    receiveData(data->sck_tcp, data->buffer);
    deserializeGrid(data->buffer, data->serverPlayer.myGrid[0]);
    */


    while (determineWinner(data->clientPlayer.myGrid[0], data->serverPlayer.myGrid[0]) == none){
        //TODO wait for your turn
        /*
        printf("Wait for another Player for his turn!\n");
        read(data->sck_tcp, data->buffer->data, 10);
        printf("Received %s from server\n", data->buffer->data);

        while(strcmp(data->buffer->data, "turn") != 0){
            read(data->sck_tcp, data->buffer->data, 10);
        }
        */


        pthread_mutex_lock(&data->mutex);

        y = GRID_SIZE / 2;
        x = GRID_SIZE / 2;



        while (true){
            system("clear");
            printf("#### PHASE TWO: BATTLE !!!\r\n");
            //copy opposites players grid for target placement
            memcpy(temp_grid, data->serverPlayer.myGrid, GRID_SIZE * GRID_SIZE * sizeof(int));
            temp_grid[x][y] = target;
            writeGrid(data->clientPlayer.myGrid[0], temp_grid[0]);
            //place a shot

            printf("Your turn! Choose tile to attack !\r\n");
            switch (getchar()) {
                case 'p':

                    //process target placement and send to server
                    serializeAttack(x,y, data->buffer);
                    sendData(data->sck_tcp, data->buffer);

                    y = GRID_SIZE / 2;
                    x = GRID_SIZE / 2;
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
        pthread_mutex_unlock(&data->mutex);

        printf("Waiting for turn!\r\n");
        receiveData(data->sck_tcp, data->buffer);

        while(strcmp(data->buffer->data, "turn")!=0){
            receiveData(data->sck_tcp, data->buffer);
        }
        pthread_mutex_lock(&data->mutex);
        pthread_cond_signal(&data->update);
        pthread_cond_wait(&data->play, &data->mutex);
        pthread_mutex_unlock(&data->mutex);
    }

    pthread_mutex_lock(&data->mutex);

    data->gameEnd = true;

    pthread_mutex_unlock(&data->mutex);




    //while()) {
    //}
    return NULL;
}

int main() {

    initscr();
    noecho();
    system("clear");
    /*
    //socket descriptor
    int sck_tcp;
    sck_tcp = createConnection(SERV_ADDR, PORT);
    if (sck_tcp < 0) {
        perror("Connecting to server failed");
        return 2;
    }
    */
    //thread data
    struct thread_data data;
    CHAR_BUFFER buf;
    data.buffer = &buf;
    thread_data_init(&data);

    if (data.sck_tcp < 0) {
        perror("Connection was not established!!");
        return 5;
    }

    pthread_t game;
    pthread_t update;

    pthread_create(&game, NULL, play, &data);
    pthread_create(&update, NULL, updateGameData, &data);

    pthread_join(game, NULL);
    pthread_join(update, NULL);


    thread_data_destroy(&data);
    endwin();

    return 0;
}