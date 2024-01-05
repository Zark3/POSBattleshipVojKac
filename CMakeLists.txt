cmake_minimum_required(VERSION 3.18)
project(POSBattleshipVojKac C)

set(CMAKE_C_STANDARD 11)

add_executable(POSBattleshipVojKac main.c
        grid.h
        grid.c
        player.c
        player.h
)

target_link_libraries(POSBattleshipVojKac curses)
