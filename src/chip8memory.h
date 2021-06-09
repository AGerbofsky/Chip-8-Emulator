
#ifndef CHIP8MEMORY_H
#define CHIP8MEMORY_H

#include "config.h"

struct chip8_memory
{
    // LOOK INTO CALLOC!!!!
    byte memory[CHIP8_MEMORY_SIZE];
};

void chip8_memory_set(struct chip8_memory* memory, int index, byte val);
byte chip8_memory_get(struct chip8_memory* memory, int index);
word chip8_memory_get_word(struct chip8_memory* memory, int index);

#endif