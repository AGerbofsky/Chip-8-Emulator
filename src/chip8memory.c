
#include "chip8memory.h"
#include <assert.h>

static void chip8_memory_is_in_bounds(int index)
{
    assert(index >= 0 && index < CHIP8_MEMORY_SIZE);
}

void chip8_memory_set(struct chip8_memory * memory, int index, byte val)
{
    chip8_memory_is_in_bounds(index);
    memory->memory[index] = val;
}

byte chip8_memory_get(struct chip8_memory * memory, int index)
{
    chip8_memory_is_in_bounds(index);
    return memory->memory[index];
}

word chip8_memory_get_word(struct chip8_memory* memory, int index)
{
    // Fetch the first instruction, at index.
    byte byte1 = chip8_memory_get(memory, index);
    // Fetch the next instruction, at the index of the next byte.
    byte byte2 = chip8_memory_get(memory, index + 1);
    // This multiplexes the two bytes into a word
    return byte1 << 8 | byte2;
}