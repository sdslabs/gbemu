#pragma once
#include "types.h"
#include "cpu.h"
#include "mmap.h"
#include "graphics.h"

// GBE stands for GameBoyEmulator

class GBE
{
private:
	// The GameBoy screen
	// 160x144 screen resolution withing a 256x224 border
	// The original GameBoy supported 4 colors
	// Pulled from https://gbdev.io/pandocs/Specifications.html
	// We might use an enum for the colors
	// Will upgrade it later for RGB colors of GBC and GBA
	// Using Byte gbe_graphicsfor now

	bool debug_mode;
	Byte screenData[160][144];

	// Pointer to CPU
	CPU* gbe_cpu;

	// Pointer to the MemoryMap
	MemoryMap* gbe_mMap;

	// Pointer to the Graphics
	PPU* gbe_graphics;

	// File pointer for Boot ROM
	FILE* bootROM;

	// File pointer for game ROM
	FILE* gameROM;

	// Placeholder event for input handling
	SDL_Event* event;

	// Update function of the GBE
	// Will be called every frame
	// GB has 59.73 frames per second
	void update();

	// cycle counter of the gameboy
	// used by CPU, PPU, APU so declared here
	static int s_Cycles;

	// Copy the boot ROM to first 256 bytes of gameROM
	// execute it and then remove it
	void executeBootROM();

	// Debug intterupt handler
	void debug_int();

	// Poll Inputs
	bool pollEvents();

	// Get Value of Registers 
	void getValueOfRegister(char registerName);

public:
	// Constructor
	// Initializes the CPU
	GBE();

	// Returns the CPU
	CPU* getCPU() { return gbe_cpu; }
};