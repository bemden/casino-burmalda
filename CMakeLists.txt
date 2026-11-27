cmake_minimum_required(VERSION 3.13)
project(BurmaldaCasino LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(BurmaldaCasino
    src/main.cpp
    src/Player.cpp
    src/Minigames.cpp
    src/Story.cpp
    src/Game.cpp
    src/UI.cpp
)

target_include_directories(BurmaldaCasino PRIVATE include)
