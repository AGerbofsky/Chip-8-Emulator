
#ifndef CHIP8REGISTERS_H
#define CHIP8REGISTERS_H

#include "config.h"

struct chip8_registers
{
    byte V[CHIP8_TOTAL_DATA_REGISTERS];     // Imprelements 0x00
    byte SP;                                // 8-bit stack pointer.
    word I;                                 // The I register is a 16-bit register used to store memory addresses.
    word PC;                                // 16-bit program counter.

    byte delay_timer;
    byte sound_timer;

};

#endif