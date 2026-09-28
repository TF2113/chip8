#ifndef CHIP_8_H
#define CHIP_8_H

unsigned short opcode; // 2 byte opcode
unsigned short stack[16];
unsigned short stack_pointer;

unsigned char memory[4096]; // 4KB memory

unsigned char V[16]; // 15 registers + 1 'carry flag'
unsigned short I; // index
unsigned short pc; // program counter

unsigned char keypad[16];
unsigned char gfx[64 * 32]; // 2048 pixel count 
unsigned char delay_timer;
unsigned char sound_timer;

#endif

