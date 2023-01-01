#pragma once
#include "types.h"
#include "cpu.h"

// GBE stands for GameBoyEmulator

class GBE
{
private:
	// Work RAM
	// Pulled from https://gbdev.io/pandocs/Specifications.html
	// Would later just be a pointer to the relevant ROM's WRAM bank

	Byte workRam[0x2000];

	// Visual RAM
	// Pulled from https://gbdev.io/pandocs/Specifications.html
	// Would later just be a pointer to the relevant ROM's VRAM bank

	Byte visualRam[0x2000];

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