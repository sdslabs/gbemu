#include "ppu.h"

void PPU::render()
{
}

// Get VRAM from MemoryMap and render it to the screen
void PPU::UpdateGraphics()
{
}

Ref<Byte> PPU::getTileData(Word address)
{
	Ref<Byte> data(new Byte[16]);
	for (int i = 0; i < 16; i++)
	{
		(data.get())[i] = mMap->readMemory(address + i);
	}
	return data;
}
