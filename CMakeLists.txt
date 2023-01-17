cmake_minimum_required(VERSION 3.22)
project(Rubicon CXX)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wextra -Wpedantic)
#add_link_options(-fsanitize=address -static-asan)

add_library(imgui
        third_party/imgui/imgui.cpp
        third_party/imgui/imgui_demo.cpp
        third_party/imgui/imgui_draw.cpp
        third_party/imgui/imgui_tables.cpp
        third_party/imgui/imgui_widgets.cpp
        third_party/imgui/imgui_impl_sdl.cpp
        third_party/imgui/imgui_impl_sdlrenderer.cpp
        )

add_executable(rubicon src/main.cpp)
target_precompile_headers(rubicon PRIVATE src/pch.h)
target_compile_definitions(rubicon PRIVATE "$<$<CONFIG:DEBUG>:_GLIBCXX_ASSERTIONS>")
target_include_directories(rubicon PUBLIC third_party)
target_include_directories(rubicon INTERFACE src)
target_link_libraries(rubicon PRIVATE imgui X11 SDL2 SDL2_image)

