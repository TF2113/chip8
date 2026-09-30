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
SDL_Surface *screenSurface = NULL;

bool init_sdl(void) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialise! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool create_window(void) {
    window =
        SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * PIXEL_SCALE,
                         SCREEN_HEIGHT * PIXEL_SCALE, SDL_WINDOW_SHOWN);
    if(window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    screenSurface = SDL_GetWindowSurface(window);
    if(screenSurface == NULL) {
        printf("Could not get window surface: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        window = NULL;
        return false;
    }

    SDL_FillRect(screenSurface, NULL,
                 SDL_MapRGB(screenSurface->format, 0, 0, 0));
    SDL_UpdateWindowSurface(window);

    return true;
}

void drawGraphics() {
    for(int y = 0; y < SCREEN_HEIGHT; y++) {
        for(int x = 0; x < SCREEN_WIDTH; x++) {
            if(gfx[y * SCREEN_WIDTH + x] != 0) {
                SDL_Rect pixel = {x * PIXEL_SCALE, y * PIXEL_SCALE, PIXEL_SCALE,
                                  PIXEL_SCALE};
                SDL_FillRect(screenSurface, &pixel,
                             SDL_MapRGB(screenSurface->format, 255, 255, 255));
            }
        }
    }
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

    if(!create_window()) {
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

        usleep(16666);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}