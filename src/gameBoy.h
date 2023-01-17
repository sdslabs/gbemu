#pragma once
#include "types.h"
#include "cpu.h"
#include "mmap.h"

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
	// Using Byte for now

	Byte screenData[160][144];

	// Pointer to CPU
	CPU* gbe_cpu;

	// Pointer to the MemoryMap
	MemoryMap* gbe_mMap;

	// File pointer for Boot ROM
	FILE* bootROM;

	// File pointer for game ROM
	FILE* gameROM;

	// Update function of the GBE
	// Will be called every frame
	// GB has 59.73 frames per second
	void update();

public:
	// Constructor
	// Initializes the CPU
	GBE();

	// Returns the CPU
	CPU* getCPU() { return gbe_cpu; };
};