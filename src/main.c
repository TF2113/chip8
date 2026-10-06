#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "chip8.h"

// display sizing
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;
const int PIXEL_SCALE = 10; // increase scale for modern displays

// SDL declarations
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

// timing values
const int INSTRUCTIONS_PER_CYCLE_IN_US =
    1429; // 700 instructions per second in microseconds
const int TIMERS_DECREMENT_COUNTER =
    12; // 60 decrements per second = (16666us / 1429us = 11.66, rounded up for
        // simplicity)
int chip8_cycle_counter = 0;

bool init_sdl(void) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialise! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool create_window_and_renderer(void) {
    if(SDL_CreateWindowAndRenderer(SCREEN_WIDTH * PIXEL_SCALE,
                                   SCREEN_HEIGHT * PIXEL_SCALE, 0, &window,
                                   &renderer) != 0) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(renderer, 9, 56, 49, 255);
    SDL_RenderClear(renderer);

    return true;
}

void drawGraphics() {
    for(int y = 0; y < SCREEN_HEIGHT; y++) {
        for(int x = 0; x < SCREEN_WIDTH; x++) {
            if(gfx[y * SCREEN_WIDTH + x] != 0) {
                SDL_Rect pixel = {x * PIXEL_SCALE, y * PIXEL_SCALE, PIXEL_SCALE,
                                  PIXEL_SCALE};
                SDL_SetRenderDrawColor(renderer, 234, 151, 56, 255);
                SDL_RenderFillRect(renderer, &pixel);
                
            }
        }
    }
    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(window);
}

int main(int argc, char *argv[]) {

    if(argc != 2) {
        printf("Usage: ./build/chip8 {ROM_PATH}");
        return 1;
    }

    chip8_initialise();

    if(!load_rom(argv[1])) {
        return 1;
    }

    if(!init_sdl()) {
        return 1;
    }

    if(!create_window_and_renderer()) {
        SDL_Quit();
        return 1;
    }

    SDL_Event event;
    int quit = 0;
    while(!quit) {
        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                quit = 1;
            }
        }
        chip8_cycle();

        if(drawFlag) {
            drawFlag = false;
            drawGraphics();
        }

        usleep(INSTRUCTIONS_PER_CYCLE_IN_US);
        chip8_cycle_counter++;

        if(chip8_cycle_counter == TIMERS_DECREMENT_COUNTER) {
            decrement_timers();
            chip8_cycle_counter = 0;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}