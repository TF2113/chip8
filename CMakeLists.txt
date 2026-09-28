cmake_minimum_required(VERSION 3.28.3)

project(chip8)

find_package(SDL2 REQUIRED)

add_executable(chip8)

target_sources(chip8
    PRIVATE
        src/main.c
)

target_link_libraries(chip8 PRIVATE SDL2::SDL2)