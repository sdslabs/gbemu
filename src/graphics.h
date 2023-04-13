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

	// Source and Destination rects to enable scrolling over texture
	SDL_Rect* source;
	SDL_Rect* dest;

	// renderArray to be converted to texture
	// stores 4 copies of texture for wrapping of screen
	color renderArray[160 * 144];

	MemoryMap* mMap;

	// LCDC 0th bit is the LCD enable flag
	bool isEnabled;

	// LCDC 3rd bit is the BG and Window Tile Data Select flag
	Word bgTileDataAddr;

	// LCDC 4th bit is the BG Tile Map Display Select flag
	Word bgTileMapAddr;

	// BGP register is the BG Palette Data
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

	// Current line being rendered
	int currentLine;

	// PPU Mode
	Byte ppuMode;

	// PPU Mode Clocks
	// Mode 0: 204 cycles
	// Mode 1: 456 cycles
	// Mode 2: 80 cycles
	// Mode 3: 172 cycles
	int modeClocks[4] = { 204, 456, 80, 172 };

	// Current PPU Mode Clock
	int currentClock;

	// Scanline Rendered Flag
	bool scanlineRendered;

	enum PPU_MODES
	{
		HBLANK,
		VBLANK,
		OAM,
		TRANSFER
	};

public:
	PPU();
	bool init();
	bool pollEvents();
	void load();
	void renderScanline(Byte line);
	void close();
	void setMemoryMap(MemoryMap* m) { mMap = m; }
	void executePPU(int cycles);
	Byte getPPUMode() { return ppuMode; }
};