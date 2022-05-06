cmake_minimum_required(VERSION 3.22)
project(rubicon C)

set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-function)

add_library(game STATIC
        src/board.c
        src/update_game.c
        src/node.c
        src/mcts.c
        )
target_include_directories(game PUBLIC third_party)
target_include_directories(game INTERFACE src)

add_executable(rubicon src/linux_main.c src/linux.h)
target_link_libraries(rubicon PRIVATE game)
target_link_libraries(rubicon PRIVATE X11 m)
