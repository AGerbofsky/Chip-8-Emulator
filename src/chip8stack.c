
#include <assert.h>

#include "chip8stack.h"
#include "chip8.h"

static void chip8_stack_in_bounds(struct chip8* chip8)
{
    assert(chip8->registers.SP < CHIP8_TOTAL_STACK_DEPTH);
}

void chip8_stack_push(struct chip8* chip8, word val)
{
    chip8_stack_in_bounds(chip8);
    chip8->stack.stack[chip8->registers.SP] = val;
    chip8->registers.SP += 1;
}

word chip8_stack_pop(struct chip8* chip8)
{
    chip8->registers.SP -= 1;
    chip8_stack_in_bounds(chip8);
    word val = chip8->stack.stack[chip8->registers.SP];
    return val;
}