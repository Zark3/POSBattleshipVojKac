cmake_minimum_required(VERSION 3.18)
project(POSBattleshipVojKac C)

set(CMAKE_C_STANDARD 11)

add_executable(POSBattleshipVojKac #main.c
        #client.c
        #client.h
        server.c
        grid.h
        grid.c
        player.c
        player.h
        pos_sockets/char_buffer.c
        pos_sockets/char_buffer.h
        pos_sockets/active_socket.c
        pos_sockets/active_socket.h
        pos_sockets/linked_list.h
        pos_sockets/passive_socket.c
        pos_sockets/passive_socket.h
)

target_link_libraries(POSBattleshipVojKac curses pthread)
