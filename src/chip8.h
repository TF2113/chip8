#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdbool.h>

extern unsigned char keypad[];
extern unsigned char gfx[];
extern bool drawFlag;

void chip8_initialise(void);
bool load_rom(const char *file_path);
void chip8_cycle();
void decrement_timers();

#endif
