
#ifndef CHIP8STACK_H
#define CHIP8STACK_H

#include "config.h"

struct chip8;

struct chip8_stack
{
    word stack[CHIP8_TOTAL_STACK_DEPTH];
};

void chip8_stack_push(struct chip8 *chip8, word val);
word chip8_stack_pop(struct chip8 *chip8);

#endif