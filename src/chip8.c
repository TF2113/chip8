#include "chip8.h"
#include "stack.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned short opcode;      // 2 byte opcode
static unsigned char memory[4096]; // 4KB memory

static unsigned char V[16]; // 15 registers + 1 'carry flag'
static unsigned short I;    // index
static unsigned short pc;   // program counter

static unsigned char delay_timer;
static unsigned char sound_timer;

bool drawFlag;
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

Stack stack;
unsigned short prevAddr;

unsigned short VX;
unsigned short VY;

void chip8_initialise() {
    pc = 0x200;
    opcode = 0;
    I = 0;

    create_stack(&stack);

    // clear display
    for(int i = 0; i < 2048; i++) {
        gfx[i] = 0;
    }

    // clear stack
    for(int i = 0; i < 16; i++) {
        stack.element[i] = 0;
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

    if(rom_size > 3232 ||
       rom_size <= 0) { // 3232 is 4KB max size minus 512 reserved space and
                        // space for display refresh, variables, stack etc
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

void chip8_cycle() {

    // fetch opcode
    opcode = memory[pc] << 8 |
             memory[pc + 1]; // opcodes are 2 bytes, merge current byte and next
                             // byte with OR operation

    // decode opcode
    switch(opcode & 0xF000) { // checks first digit

    case 0x0000: // first hex digit is 0
        switch(opcode) {

        case 0x00E0: // 0x00E0: Clear Screen

            for(int i = 0; i < 2048; i++) {
                gfx[i] = 0;
            }

            drawFlag = true; // set display to be redrawn
            pc += 2;
            break;

        case 0x00EE: // 0x00EE: Return from subroutine
            pop(&stack, &prevAddr);
            pc = prevAddr + 2;
            break;
        }
        break;

    case 0x1000: // 0x1NNN: Jump
        pc = opcode & 0x0FFF;
        break;

    case 0x2000: // 0x2NNN: Call subroutine at NNN
        push(&stack, pc);
        pc = opcode & 0x0FFF;
        break;

    case 0x3000: // 0x3XNN: Skip instruction if VX == NN
        if((V[(opcode & 0x0F00) >> 8]) == (opcode & 0x00FF)) {
            pc += 2;
        }
        pc += 2;
        break;

    case 0x4000: // 0x4XNN: Skip instruction if VX != NN
        if((V[(opcode & 0x0F00) >> 8]) != (opcode & 0x00FF)) {
            pc += 2;
        }
        pc += 2;
        break;

    case 0x5000: // 0x5XY0: Skip instruction if VX == VY
        if((V[(opcode & 0x0F00) >> 8]) == (V[(opcode & 0x00F0) >> 4])) {
            pc += 2;
        }
        pc += 2;
        break;

    case 0x6000: // 0x6XNN: Set Register V[X] to NN
        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        pc += 2;
        break;

    case 0x7000: // 0x7XNN: Add NN to register V[X]
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        pc += 2;
        break;

    case 0x8000: // 0x8 series of instructions to perform logical/arithmetic
                 // functions
        VX = V[(opcode & 0x0F00) >> 8];
        VY = V[(opcode & 0x00F0) >> 4];
        switch(opcode & 0xF00F) {

        case 0x8000: // 0x8XY0: Set VX to value of VY
            VX = VY;
            V[(opcode & 0x0F00) >> 8] = VX;
            pc += 2;
            break;

        case 0x8001: // 0x8XY1: Set VX to binary OR of VX and VY
            VX |= VY;
            V[(opcode & 0x0F00) >> 8] = VX;
            pc += 2;
            break;

        case 0x8002: // 0x8XY2: Set VX to binary AND of VX and VY
            VX &= VY;
            V[(opcode & 0x0F00) >> 8] = VX;
            pc += 2;
            break;

        case 0x8003: // 0x8XY3: Set VX to binary XOR of VX and VY
            VX ^= VY;
            V[(opcode & 0x0F00) >> 8] = VX;
            pc += 2;
            break;

        case 0x8004: // 0x8XY4: Add value of VY to VX, if VX overflows set VF =
                     // 1
            VX += VY;
            V[(opcode & 0x0F00) >> 8] = VX;

            V[0xF] = 0;
            if((VX + VY) > 255) {
                V[0xF] = 1;
            }
            pc += 2;
            break;

        case 0x8005: // 0x8XY5: Subtract value of VY from VX and store in VX
            if(VX >= VY) {
                VX -= VY;
                V[(opcode & 0x0F00) >> 8] = VX;
                V[0xF] = 1;
            } else {
                VX -= VY;
                V[(opcode & 0x0F00) >> 8] = VX;
                V[0xF] = 0;
            }

            pc += 2;
            break;

        case 0x8006: // 0x8XY6: Set VX = VY and shift the value of VX one bit to
                     // the right
            VX = VY >> 1;
            V[(opcode & 0x0F00) >> 8] = VX;

            V[0xF] = 0;
            if((VY & 1) == 1) {
                V[0xF] = 1;
            }
            pc += 2;
            break;

        case 0x8007: // 0x8XY7: Subtract value of VX from VY and store in VX
            VX = VY - VX;
            V[(opcode & 0x0F00) >> 8] = VX;

            V[0xF] = 0;
            if(VY >= VX) {
                V[0xF] = 1;
            }
            pc += 2;
            break;

        case 0x800E: // 0x8XYE:  Set VX = VY and shift the value of VX one bit
                     // to the left
            VX = VY << 1;
            V[(opcode & 0x0F00) >> 8] = VX;

            V[0xF] = 0;
            if((VY >> 7) == 1) {
                V[0xF] = 1;
            }
            pc += 2;
            break;
        }
        break;

    case 0x9000: // 0x9XY0: Skip instruction if VX != VY
        if((V[(opcode & 0x0F00) >> 8]) != (V[(opcode & 0x00F0) >> 4])) {
            pc += 2;
        }
        pc += 2;
        break;

    case 0xA000: // 0xANNN: Set index to NNN
        I = opcode & 0x0FFF;
        pc += 2;
        break;

    case 0xD000: // 0xDXYN: Draw display (Credit to James Griffin, I was lost in
                 // the sauce (https://github.com/JamesGriffin/CHIP-8-Emulator))
        unsigned short xCord = V[(opcode & 0x0F00) >> 8];
        unsigned short yCord = V[(opcode & 0x00F0) >> 4];
        unsigned short rowCount = opcode & 0x000F;
        unsigned short currentPixel;
        V[0xF] = 0;

        for(int i = 0; i < rowCount; i++) {
            currentPixel = memory[I + i];
            for(int j = 0; j < 8; j++) {
                if((currentPixel & (0x80 >> j)) != 0) {
                    if(gfx[(xCord + j) + ((yCord + i) * 64)] == 1) {
                        V[0xF] = 1;
                    }
                    gfx[(xCord + j) + ((yCord + i) * 64)] ^= 1;
                }
            }
        }

        drawFlag = true;
        pc += 2;
        break;

    case 0xF000: // Pair of instruction store or load registers from memory
        unsigned short end = (opcode & 0x0F00) >> 8;
        switch(opcode & 0xF0FF) {

        case 0xF007: // 0xFX07: Set VX to current value of delay timer
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            pc += 2;
            break;

        case 0xF015: // 0xFX15: Set delay timer to current value of VX
            delay_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0xF018: // 0xFX18: Set sound timer to current value of VX
            sound_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0xF01E: // 0xFX1E: Add value of VX to I
            I += V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0xF029: // 0xFX29: Set I to the fontset address of the hex char
                     // stored in VX
            I = V[(opcode & 0x0F00) >> 8] * 5;
            pc += 2;
            break;

        case 0xF033: // 0xFX33: Convert value of VX into 3 digits, e.g 255 in 2,
                     // 5, 5, and store into memory starting at I
            memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 10);

            pc += 2;
            break;

        case 0xF055: // 0xFX55: Store registers starting from V0 to VX into
                     // memory
            for(int i = 0; i <= end; i++) {
                memory[I + i] = V[i];
            }

            pc += 2;
            break;

        case 0xF065: // 0xFX65: Load registers into V0 to VX from memory
            for(int i = 0; i <= end; i++) {
                V[i] = memory[I + i];
            }

            pc += 2;
            break;
        }
        break;

    default:
        printf("Unknown Opcode\n");
    }
}

void decrement_timers() {
    if(delay_timer > 0)
        --delay_timer;

    if(sound_timer > 0) {
        printf("BEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEP!\n");
        --sound_timer;
    }
}
