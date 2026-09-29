#include "chip8.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned short opcode; // 2 byte opcode
static unsigned short stack[16];
static unsigned short stack_pointer;
static unsigned char memory[4096]; // 4KB memory

static unsigned char V[16]; // 15 registers + 1 'carry flag'
static unsigned short I;    // index
static unsigned short pc;   // program counter

static unsigned char delay_timer;
static unsigned char sound_timer;

unsigned char keypad[16];
unsigned char gfx[64 * 32]; // 2048 pixel count

unsigned char fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_initialise() {
    pc = 0x200;
    opcode = 0;
    I = 0;
    stack_pointer = 0;

    // clear display
    for(int i = 0; i < 2048; i++) {
        gfx[i] = 0;
    }

    // clear stack
    for(int i = 0; i < 16; i++) {
        stack[i] = 0;
    }

    // clear registers
    for(int i = 0; i < 16; i++) {
        V[i] = 0;
    }

    // clear memory
    for(int i = 0; i < 4096; i++) {
        memory[i] = 0;
    }

    // load fontset into memory
    for(int i = 0; i < 80; i++) {
        memory[i] = fontset[i];
    }

    delay_timer = 60;
    sound_timer = 60;

    printf("Chip8 Successfully initialised\n");
}

bool load_rom(const char *file_path) {

    FILE *rom;

    if(file_path == NULL) {
        printf("ROM cannot be NULL\n");
        return false;
    }

    rom = fopen(file_path, "rb");

    if(rom == NULL) {
        printf("Not able to open file\n");
        return false;
    }

    if(fseek(rom, 0, SEEK_END) != 0) {
        printf("fseek failed\n");
        fclose(rom);
        return false;
    }

    long rom_size = ftell(rom);

    if(rom_size == -1L) {
        printf("ftell failed\n");
        fclose(rom);
        return false;
    }

    rewind(rom);

    if(rom_size > 512 || rom_size <= 0) {
        printf("ROM size error, max size 512b\n");
        fclose(rom);
        return false;
    }

    size_t store_rom = fread(memory + 0x200, 1, rom_size, rom);

    if(store_rom != rom_size) {
        printf("Failed to read ROM\n");
        fclose(rom);
        return false;
    }

    fclose(rom);
    return true;
}
