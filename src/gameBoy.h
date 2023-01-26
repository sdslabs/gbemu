#pragma once
#include "types.h"
#include "cpu.h"
#include "mmap.h"
#include "ppu.h"

// GBE stands for GameBoyEmulator

class GBE
{
private:
	// Pointer to CPU
	Ref<CPU> gbe_cpu;

	// Pointer to PPU
	Ref<PPU> gbe_ppu;

	// Pointer to the MemoryMap
	Ref<MemoryMap> gbe_mMap;

	// File pointer for Boot ROM
	FILE* bootROM;

	// File pointer for game ROM
	FILE* gameROM;

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

public:
	// Constructor
	// Initializes the CPU
	GBE();
};