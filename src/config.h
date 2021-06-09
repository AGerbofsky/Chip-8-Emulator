

#ifndef CONFIG_H
#define CONFIG_H

#define EMULATOR_WINDOW_TITLE   "Chip8 Emulator"

#define CHIP8_MEMORY_SIZE 0x1000    // Size of chip8 addressable memory is 4kB.
#define CHIP8_WIDTH       64        // Screen pixel width.
#define CHIP8_HEIGHT      32        // Screen pixel height.

#define CHIP8_WINDOW_MULTIPLIER  40 // Pixel scale multiplier. The instructor used 10 but we have
                                    // a 4k monitor so that won't cut it. We will use 40 instead.

#define CHIP8_TOTAL_DATA_REGISTERS  0x0010  // There are 16 GPRs in the Chip8.
#define CHIP8_TOTAL_STACK_DEPTH     0x0010  // The stack has 16-words, depth of 16 addresses.
#define CHIP8_TOTAL_KEYS            0x0010  // The Chip8 keypad has 16 keys.

#define CHIP8_DEFAULT_SPRITE_HEIGHT         0x0005  
#define CHIP8_DEFAULT_SPRITE_WIDTH          0x0008
#define CHIP8_CHARACTER_SET_LOAD_ADDRESS    0x0000  // This is the base address of the "Character-ROM".
#define CHIP8_CHARACTER_SET_MAX_ADDRESS     0x01ff  // The end of the "Character-ROM" is $01ff. Added
                                                    // for good measure, not for practicle use.
#define CHIP8_PROGRAM_MEMORY_ADDRESS        0x0200  // This is the typical starting point for Chip8 prgs.

typedef unsigned char byte;
typedef unsigned short word;

#endif