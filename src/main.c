

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <SDL2/SDL.h>

#include "chip8.h"
#include "chip8keyboard.h"

// Keyboard map for virtual keyboard.
const char keyboard_map[CHIP8_TOTAL_KEYS] = {
	SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5,
	SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_a, SDLK_b,
	SDLK_c, SDLK_d, SDLK_e, SDLK_f};

int main(int argc, const char **argv)
{
	if (argc < 2)
	{
		printf("You must provide a file to load!\n");
		return -1;
	}

	const char* filename = argv[1];
	printf("The filename to load is: %s\n", filename);

	// Actually load a file into memory.
	FILE* f = fopen(filename, "r");
	if (!f)
	{
		printf("Failed to open the file");
		return -1;
	}

	// Seek to the end of the file to establish the required buffer size. Once done, seek back
	// to the begining of the file where we will begin reading.
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	// Create a buffer for reading our program file. fread() will attempt to read as one chunk.
	char buf[size];
	int res = fread(buf, size, 1, f);
	if (res != 1)
	{
		printf("Failed to read from file");
		return -1;
	}

	// Instantization of the chip8 structure.
	struct chip8 chip8;
	chip8_init(&chip8);
	// Call the function responsible for loading the program into Chip8 program memory.
	chip8_load(&chip8, buf, size);

	chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window *window = SDL_CreateWindow(
		EMULATOR_WINDOW_TITLE,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		CHIP8_WIDTH * CHIP8_WINDOW_MULTIPLIER,
		CHIP8_HEIGHT * CHIP8_WINDOW_MULTIPLIER,
		SDL_WINDOW_SHOWN);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);

	while (1)
	{
		SDL_Event event;

		// Switch statement parses events during program execution.
		while (SDL_PollEvent(&event))
		{

			switch (event.type)
			{
			case SDL_QUIT:
				goto out;
				break;

			case SDL_KEYDOWN:
			{
				// Detect which key has been pressed.
				char key = event.key.keysym.sym;
				// Now map it to our chip8 virtual keyboard map.
				int vkey = chip8_keyboard_map(&chip8.keyboard, key);

				if (vkey != -1)
				{
					chip8_keyboard_down(&chip8.keyboard, vkey);
				}
			}

			break;

			case SDL_KEYUP:
			{
				// Detect which key has been pressed.
				char key = event.key.keysym.sym;
				// Now map it to our chip8 virtual keyboard map.
				int vkey = chip8_keyboard_map(&chip8.keyboard, key);

				if (vkey != -1)
				{
					chip8_keyboard_up(&chip8.keyboard, vkey);
				}
			}
			break;
			};
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

		for (int x = 0; x < CHIP8_WIDTH; x++)
		{
			for (int y = 0; y < CHIP8_HEIGHT; y++)
			{
				if (chip8_screen_is_set(&chip8.screen, x, y))
				{
					SDL_Rect r;

					r.x = x * CHIP8_WINDOW_MULTIPLIER;
					r.y = y * CHIP8_WINDOW_MULTIPLIER;
					r.w = CHIP8_WINDOW_MULTIPLIER;
					r.h = CHIP8_WINDOW_MULTIPLIER;

					SDL_RenderFillRect(renderer, &r);
					
				}
			}
		}

		SDL_RenderPresent(renderer);
	
		if (chip8.registers.delay_timer > 0)
		{
			// Unistd.h implements this differently so... rather than sleep(100) for windows.h
			// we will do sleep(.10) for "sleep 100 milliseconds".
			sleep(.10);
			chip8.registers.delay_timer -= 1;
		}

		// This functionality doesn't work in linux )*:
		if (chip8.registers.sound_timer > 0)
		{
			// This is all that poor Linux can do in this case...
			//printf("\a");
			sleep(.10);
			chip8.registers.sound_timer -= 1;
		}

		// At the end of each cycle, begin the instruction fetch.
		word opcode = chip8_memory_get_word(&chip8.memory, chip8.registers.PC);
		// Then when finished with those... advance the program counter by two bytes.
		chip8.registers.PC += 2;
		// Then we execute the instruction.
		chip8_exec(&chip8, opcode);

	}

out:
	SDL_DestroyWindow(window);
	return 0;
}
