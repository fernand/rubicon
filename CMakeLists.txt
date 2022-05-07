cmake_minimum_required(VERSION 3.22)
project(rubicon C)

set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall -Wextra -Wpedantic)
#add_link_options(-fsanitize=address -static-libasan)

add_executable(rubicon src/linux_main.c)
target_include_directories(rubicon PUBLIC third_party)
target_include_directories(rubicon INTERFACE src)
target_link_libraries(rubicon PRIVATE X11 m)
