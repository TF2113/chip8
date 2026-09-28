cmake_minimum_required(VERSION 3.28.3)

project(chip8)

add_executable(chip8)

target_sources(chip8
    PRIVATE
        src/chip8.c
)