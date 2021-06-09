
#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "chip8.h"

// This array contains the values of the character set's default character bit-maps.
// Each of the 16 rows contains 5 bytes that are used to "turn on" a pixel. The
// contents of these character bit maps will be stored in memory locations 
// ($0000 to $01FF). This memory range we can consider as our "Font-ROM".
const char chip8_default_character_set[] = {
    0xf0, 0x90, 0x90, 0x90, 0xf0, // Value "0"
    0x20, 0x60, 0x20, 0x20, 0x70, // Value "1"
    0xf0, 0x10, 0xf0, 0x80, 0xf0, // Value "2"
    0xf0, 0x10, 0xf0, 0x10, 0xf0, // Value "3"
    0x90, 0x90, 0xf0, 0x10, 0x10, // Value "4"
    0xf0, 0x80, 0xf0, 0x10, 0xf0, // Value "5"
    0xf0, 0x80, 0xf0, 0x90, 0xf0, // Value "6"
    0xf0, 0x10, 0x20, 0x40, 0x40, // Value "7"
    0xf0, 0x90, 0xf0, 0x90, 0xf0, // Value "8"
    0xf0, 0x90, 0xf0, 0x10, 0xf0, // Value "9"  
    0xf0, 0x90, 0xf0, 0x90, 0x90, // Value "A"
    0xe0, 0x90, 0xe0, 0x90, 0xe0, // Value "B"   
    0xf0, 0x80, 0x80, 0x80, 0xf0, // Value "C"   
    0xe0, 0x90, 0x90, 0x90, 0xe0, // Value "D"    
    0xf0, 0x80, 0xf0, 0x80, 0xf0, // Value "E"
    0xf0, 0x80, 0xf0, 0x80, 0x80  // Value "F"   
};

// Function responsible for initializing the "initial state" of the chip8 structure.
void chip8_init(struct chip8* chip8)
{
    // Sets all member variables and pointers of the chip8 structure to 0 or "NULL", at the start
    // of the application runtime.
    memset(chip8, 0, sizeof(struct chip8));
    // This memcpy copies the contents of the default character set into memory/"Font-ROM".
    memcpy(&chip8->memory.memory, chip8_default_character_set, sizeof(chip8_default_character_set));
}

// Function responsible for loading the program into the chip8's memory.
void chip8_load(struct chip8* chip8, const char* buf, size_t size)
{
    // size and CHIP8_PROGRAM_MEMORY_ADDRESS are combined because we need to take the $0200 offset
    // into account when we load in the memory. This is to avoid memory conflicts.
    assert (size + CHIP8_PROGRAM_MEMORY_ADDRESS < CHIP8_MEMORY_SIZE);
    memcpy(&chip8->memory.memory[CHIP8_PROGRAM_MEMORY_ADDRESS], buf, size);
    // Move the program counter to the address of the begining of program memory.
    chip8->registers.PC = CHIP8_PROGRAM_MEMORY_ADDRESS;
}

static void chip8_execute_extened_eight(struct chip8* chip8, word opcode)
{
    byte x   = (opcode >> 8) & 0x000f;
    byte y   = (opcode >> 4) & 0x000f;
    byte n   = opcode & 0x000f;

    word tmp = 0;

    switch(n)
    {
        // LD Vx, Vy: Store Vy in Vx
        case 0x00:
            chip8->registers.V[x] = chip8->registers.V[y];
        break;

        // OR Vx, Vy: Set Vx OR Vy
        case 0x01:
            chip8->registers.V[x] |= chip8->registers.V[y]; 
        break;

        // AND Vx, Vy: Set Vx AND Vy
        case 0x02:
            chip8->registers.V[x] &= chip8->registers.V[y];
        break;

        // XOR Vx, Vy: Set Vx XOR Vy
        case 0x03:
            chip8->registers.V[x] ^= chip8->registers.V[y];
        break;

        // ADD carry Vx = Vx + Vy: Add Vx + Vy with carry
        case 0x04:
            tmp = chip8->registers.V[x] + chip8->registers.V[y];
            chip8->registers.V[0x0f] = false;

            if(tmp > 0xff)
            {
                chip8->registers.V[0x0f] = true;
            }

            chip8->registers.V[x] = tmp;

        break;

        // SUB carry Vx, Vy: Set Vx = Vx - Vy, set VF = NOT Borrow
        case 0x05:
            chip8->registers.V[0x0f] = false;

            if (chip8->registers.V[x] > chip8->registers.V[y])
            {
                chip8->registers.V[0x0f] = true;
            }

            chip8->registers.V[x] -= chip8->registers.V[y];

        // SHR Vx {, Vy}: Set Vx = Vx SHR 1. (IFF lsb == 1 ? then VF = 1 : VF = 0. Then Vx/2)
        case 0x06:
            chip8->registers.V[0x0f] = chip8->registers.V[x] & 0b00000001;
            chip8->registers.V[x] /= 2;
        break;

        // SUBN Vx, Vy: Set Vx - Vy (IFF Vy > Vx ? VF = 1 : VF = 0)
        case 0x07:
            chip8->registers.V[0x0f] = chip8->registers.V[y] > chip8->registers.V[x];
            chip8->registers.V[x] = chip8->registers.V[y] - chip8->registers.V[x];
        break;

        // SHL Vx {, Vy}:
        case 0x0E:
            chip8->registers.V[0x0f] = chip8->registers.V[x] & 0b10000000;
            chip8->registers.V[x] *= 2;       
        break;
    }

}

static char chip8_wait_for_key_press(struct chip8 * chip8)
{
    SDL_Event event;

    while(SDL_WaitEvent(&event))
    {
        if (event.type != SDL_KEYDOWN)
            continue;

        char c = event.key.keysym.sym;
        char chip8_key = chip8_keyboard_map(&chip8->keyboard, c);

        if(chip8_key != -1)
        {
            return chip8_key;
        }
    }
}

static void chip8_execute_extended_F(struct chip8* chip8, word opcode)
{
    byte x   = (opcode >> 8) & 0x000f;
    byte y   = (opcode >> 4) & 0x000f;
    byte n   = opcode & 0x000f;

    switch(opcode & 0x00ff)
    {
        // LD Vx, DT: Load the value of Delay Timer into the Vx register
        case 0x07:
            chip8->registers.V[x] = chip8->registers.delay_timer;
        break;

        // LD Vx, K: Wait for a keypress, store the value of that keypress into the Vx register
        case 0x0A:
        {
            char pressed_key = chip8_wait_for_key_press(chip8);
            chip8->registers.V[x] = pressed_key;
        }
        break;

        // LD DT, Vx: Set delay timer = Vx
        case 0x15:
            chip8->registers.delay_timer = chip8->registers.V[x];
        break;

        // LD ST, Vx: Set delay timer = Vx
        case 0x18:
            chip8->registers.sound_timer = chip8->registers.V[x];
        break;

        // ADD I, Vx: Add the value of Vx to the value of I, then store it in I
        case 0x1E:
            chip8->registers.I += chip8->registers.V[x];
        break;

        // LD F, Vx: Set I equal to the location of the sprite for digit Vx
        case 0x29:
            chip8->registers.I = chip8->registers.V[x] * CHIP8_DEFAULT_SPRITE_HEIGHT;
        break;

        // LD B, Vx: Store BCD representation of Vx in memory locations I, I+1, and I+2
        // Note: This is why BCD takes up so much room...
        case 0x33:
        {
            byte hundreds = chip8->registers.V[x] / 100;
            byte tens = chip8->registers.V[x] / 10 % 10;
            byte units = chip8->registers.V[x] % 10;

            chip8_memory_set(&chip8->memory, chip8->registers.I  , hundreds);
            chip8_memory_set(&chip8->memory, chip8->registers.I+1, tens);
            chip8_memory_set(&chip8->memory, chip8->registers.I+2, units);
        }
        break;

        // LD [I], Vx: Store registers V0 - Vx in memory starting at location I
        case 0x55:
        {
            for (int i = 0; i <= x; i++)
            {
                chip8_memory_set(&chip8->memory, chip8->registers.I+i, chip8->registers.V[i]);
            }
        }
        break;

        // LD Vx, [I]: Read Registers V0 - Vx from memory starting at location I
        case 0x65:
        {
            for (int i = 0; i <= x; i++)
            {
                chip8->registers.V[i] = chip8_memory_get(&chip8->memory, chip8->registers.I+i);
            }
        }
        break;
    }

}

static void chip8_execute_extended(struct chip8* chip8, word opcode)
{
    word nnn = opcode & 0x0fff;
    byte x   = (opcode >> 8) & 0x000f;
    byte y   = (opcode >> 4) & 0x000f;
    byte kk  = opcode & 0x00ff;
    byte n   = opcode & 0x000f; 

    switch(opcode & 0xf000)
    {
        // JP addr: Jump to address (1nnn jp @ nnn)
        case 0x1000:
            chip8->registers.PC = nnn;
        break;

        // CALL addr: Call subroutine (2nnn call @ nnn)
        case 0x2000:
            chip8_stack_push(chip8, chip8->registers.PC);
            chip8->registers.PC = nnn;
        break;

        // SE Vx, byte: Skip the next instruction (IFF Vx = kk)
        case 0x3000:
            if (chip8->registers.V[x] == kk)
                chip8->registers.PC += 2;
        break;

        // SNE Vx, byte: Skip the next instruction (IFF Vx != kk)
        case 0x4000:
            if (chip8->registers.V[x] != kk)
                chip8->registers.PC += 2;
        break;

        // SE Vx, Vy: Skip the next instruction (IFF Vx == Vy)
        case 0x5000:
            if (chip8->registers.V[x] == chip8->registers.V[y])
                chip8->registers.PC += 2;

        // LQ Vx, byte: Set Vx = kk
        case 0x6000:
            chip8->registers.V[x] = kk;
        break;

        // ADD Vx, byte: Add kk to the value of Vx
        case 0x7000:
            chip8->registers.V[x] += kk;
        break;

        // LD Vx, Vy: Store |Vy| -> |Vx|
        case 0x8000:
            chip8_execute_extened_eight(chip8, opcode);
        break;

        // SNE Vx, Vy: Skip to next instruction IFF Vx != Vy
        case 0x9000:
            if(chip8->registers.V[x] != chip8->registers.V[y])
                chip8->registers.PC += 2;
        break;      

        // LD I, addr: Load the I register with address nnn
        case 0xA000:
            chip8->registers.I = nnn;
        break;  

        // JP V0, addr: Jump to location nnn with V0 offset
        case 0xB000:
            chip8->registers.PC = nnn + chip8->registers.V[0x00];
        break;

        // RND Vx, byte: Random byte AND kk
        case 0xC000:
            srand(clock());
            chip8->registers.V[x] = rand() % 255 && kk;
        break;

        // DRW Vx, Vy, nibble: Display n-byte sprite starting at addr I at (Vx, Vy) index.
        // Set the VF FLAGS register = collision.
        case 0xD000:
        {
            const char* sprite = &chip8->memory.memory[chip8->registers.I];
            chip8->registers.V[0x0f] = chip8_screen_draw_sprite(&chip8->screen, chip8->registers.V[x], chip8->registers.V[y], sprite, n);
        }
        break;

        case 0xE000:
        {
            switch(opcode & 0x00ff)
            {
                // SKP Vx: Skip next instruction if key with value of Vx is pressed
                case 0x9e:
                    if(chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x]))
                        chip8->registers.PC += 2;
                break;

                // SKNP Vx: Skip next instruction if key with value of Vx is not pressed
                case 0xa1:
                    if(!chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x]))
                        chip8->registers.PC += 2;
                break;
            }
        }
        break;

        case 0xF000:
        {
            chip8_execute_extended_F(chip8, opcode);
        }
        break;
        
    }
}

void chip8_exec(struct chip8* chip8, word opcode)
{
    switch(opcode)
    {
        // CLS: Clear Screen
        case 0x00E0:
            chip8_screen_clear(&chip8->screen);
        break;

        // RET: Return from Subroutine
        case 0x00EE:
            chip8->registers.PC = chip8_stack_pop(chip8);
        break;

        // Special Case Instructions: For instructions that require bitwise operations.
        default:
            chip8_execute_extended(chip8, opcode);
        

    }
}