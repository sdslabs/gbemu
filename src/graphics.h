#pragma once
#include "types.h"
#include "mmap.h"
#include <stdio.h>
#include <algorithm>
#include <SDL.h>

class PPU
{
private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Event* event;
	color renderArray[256 * 256];
	color nullArray[256 * 256];

	MemoryMap* mMap;
	bool isEnabled;
	Word bgTileMapAddr;
	Word bgTileDataAddr;
	Byte bgPalette;

	// The GameBoy screen
	// 160x144 screen resolution withing a 256x224 border
	// The original GameBoy supported 4 colors
	// Pulled from https://gbdev.io/pandocs/Specifications.html
	const int SCREEN_WIDTH = 160;
	const int SCREEN_HEIGHT = 144;

	// Color Mapping for background
	color bg_colors[4] = { 0x9BBC0FFF, 0x8BAC0FFF, 0x306230FF, 0x0F380FFF };

	// Color Mapping for objects
	// NOTE: 0 is transparent
	// indices 1, 2, 3 are the actual colors and will be populated later
	color obj_colors[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

public:
	PPU();
	bool init();
	bool pollEvents();
	void load();
	void close();
	void setMemoryMap(MemoryMap* m) { mMap = m; }
};