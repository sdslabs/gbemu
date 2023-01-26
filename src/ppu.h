#pragma once
#include "mmap.h"

class PPU
{
private:
	Ref<MemoryMap> mMap;

	// The GameBoy screen
	// 160x144 screen resolution withing a 256x224 border
	// The original GameBoy supported 4 colors
	// Pulled from https://gbdev.io/pandocs/Specifications.html
	// We might use an enum for the colors
	// Will upgrade it later for RGB colors of GBC and GBA
	// Using Byte for now
	Byte screenData[160][144];

	void render();

	Ref<Byte> getTileData(Word address);

public:
	PPU() = default;

	inline void SetMemory(MemoryMap& memory) { mMap = MakeRef<MemoryMap>(memory); };

	void UpdateGraphics();
};
