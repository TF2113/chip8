# Chip 8 Emulator/Interpreter

Chip8 implemented in C, using SDL2 library for handling the window and inputs. <br>
Few issues remain, flickering display when sprites in motion, collison detection is wonky. <br> 
Built over ~10 days to learn basic emulation development and further low-level development knowledge without AI assistance, only online resources and my own brain.

<br>

# Build and Run

1. Install SDL2<br>
2. Build with `cmake -S . -B build && cmake --build build` <br>
3. Run with `./build/chip8 ./roms/{ROM-NAME}`

<br>

# Screenshots

![alt text](./docs/ibm.png)
![alt text](./docs/flags.png)
![alt text](./docs/breakout.png)
![alt text](./docs/pong.png)

# Credits:

Tobias V.I Langhoff for the great guide<br>
https://tobiasvl.github.io/blog/write-a-chip-8-emulator/<br>
https://github.com/tobiasvl

Test Roms from @Timendus <br>
https://github.com/Timendus/chip8-test-suite

Games from @Kripod <br>
https://github.com/kripod/chip8-roms