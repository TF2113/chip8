#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdbool.h>

extern unsigned char keypad[];
extern unsigned char gfx[];

void chip8_initialise(void);
bool load_rom(const char *file_path);

#endif
