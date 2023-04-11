#include "types.h"
#include "graphics.h"

PPU::PPU()
{
	// Initialize members
	window = nullptr;
	renderer = nullptr;
	texture = nullptr;
	isEnabled = false;
	mMap = nullptr;
	bgTileDataAddr = 0x0000;
	bgTileMapAddr = 0x0000;
	bgPalette = 0x00;
	currentLine = 0x00;
	ppuMode = 0x02;
	event = new SDL_Event();
	source = new SDL_Rect({ 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT });
	dest = new SDL_Rect({ 0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2 });

	ppuMode = 0;
	currentClock = modeClocks[ppuMode];
	scanlineRendered = false;

	// Fill renderArray initially with white (lightest color in palette)
	std::fill(renderArray, renderArray + (256 * 256 * 4), bg_colors[0]);
}

bool PPU::init()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// Set hint to use hardware acceleration
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Hardware Acceleration not enabled! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// Set hint for VSync
	if (!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"))
	{
		printf("VSync not enabled! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// Create window and renderer
	if (!(window = SDL_CreateWindow("GameBoy Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, SDL_WINDOW_SHOWN)))
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	if (!(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)))
	{
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// Evaluate LCDC register
	Byte LCDC = mMap->getRegLCDC();

	isEnabled = (LCDC & 0x80);
	bgTileMapAddr = (LCDC & 0x04) ? 0x9C00 : 0x9800;
	bgTileDataAddr = (LCDC & 0x08) ? 0x8000 : 0x8800;

	// Evaluate Background Palette register
	bgPalette = mMap->getRegBGP();

	// Create a placeholder texture
	// 512x512 to have 4 copies of tilemap
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 256 * 2, 256 * 2);

	// Render the texture
	SDL_UpdateTexture(texture, NULL, renderArray, 512 * 4);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, source, dest);
	SDL_RenderPresent(renderer);
	return true;
}

// Poll Events to check for inputs
// And process them
bool PPU::pollEvents()
{
	while (SDL_PollEvent(event))
	{
		if (event->type == SDL_KEYDOWN)
		{
			mMap->writeMemory(0xFF02, 0x81);
			printf("Key pressed: %c\n", event->key.keysym.sym);
			if (event->key.keysym.sym == SDLK_ESCAPE)
				exit(0);
		}
	}
	return false;
}

void PPU::load()
{
	// Evaluate LCDC register
	Byte LCDC = mMap->getRegLCDC();

	isEnabled = (LCDC & 0x80);
	// bgTileMapAddr = (LCDC & 0x04) ? 0x9C00 : 0x9800;
	bgTileMapAddr = 0x9800;
	// bgTileDataAddr = (LCDC & 0x08) ? 0x8000 : 0x8800;
	bgTileDataAddr = 0x8000;

	// Read background palette register
	bgPalette = mMap->getRegBGP();
	// bgPalette = 0xE4;

	Word tilenum;
	Byte pixelCol;

	// Filling pixel array
	// Going over each pixel on screen
	// And filling it with the correct color
	// If LCDC.4 is set, then the background tile map uses $8000 method and unsigned addressing
	// If LCDC.4 is not set, then the background tile map uses $8800 method and signed addressing
	// Each tile has 8x8 pixels, and each pixel has a color ID of 0 to 3
	// Each tile occupies 16 bytes, where each line is represented by 2 bytes
	// For each line, the 1st byte specifies the LSB of the color ID of each pixel, and the 2nd byte specifies the MSB.
	// The color numbers are translated into gray shades depending on the current palette

	// First we calculate the tile number of the tile that the pixel is in
	// To do that, we divide the pixel's x and y coordinates by 8 (floor division)
	// Then we multiply the resultant y by 32 (the number of tiles in a row) (256 pixels / 8 pixels per tile = 32 tiles)
	// Then we add the resultant x which gives us the tile number we must check for data
	// Here i is y and j is x

	// Now, using the tile number, we can calculate the address of the tile data by multiplying the tile number by 16 and adding it to the tile data address
	// The tile data address is either 0x8000 or 0x8800 depending on LCDC.4 for Background
	// If the tile data address is 0x8000, then the tile number is unsigned, else signed at 0x8800
	// Now, depending on the row of pixel, we find which 2 bytes of data to use out of the 16 in pixel data, by taking remainder from 8 and choosing the pair of bytes
	// Then, we find the color ID of the pixel by taking the bit at the position of the pixel in the row (7 - (j % 8)) and shifting it to the LSB for both the bytes in the pair
	// 7-(j % 8) will give us the 7th bit for j = 0, 6th bit for j = 1, and so on, and the last bit for j = 8. Exactly the bit we need from both bytes in pair
	// Adding these LSBs will give us the pixel color we want

	// Source: https://gbdev.io/pandocs/Tile_Data.html

	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			tilenum = (*mMap)[bgTileMapAddr + ((i / 8) * 32) + (j / 8)];
			if (bgTileDataAddr == 0x8800)
			{
				pixelCol = ((*mMap)[bgTileDataAddr + (tilenum * 0x10) + (i % 8 * 2)] >> (7 - (j % 8)) & 0x1) + ((*mMap)[bgTileDataAddr + (tilenum * 0x10) + (i % 8 * 2) + 1] >> (7 - (j % 8)) & 0x1) * 2;
			}
			else
			{
				pixelCol = ((*mMap)[bgTileDataAddr + ((SWord)tilenum * 0x10) + (i % 8 * 2)] >> (7 - (j % 8)) & 0x1) + (((*mMap)[bgTileDataAddr + ((SWord)tilenum * 0x10) + (i % 8 * 2) + 1] >> (7 - (j % 8)) & 0x1) * 2);
			}

			renderArray[(i * 512) + j] = bg_colors[(bgPalette >> (pixelCol * 2)) & 0x3];
			renderArray[(i * 512) + (256 + j)] = bg_colors[(bgPalette >> (pixelCol * 2)) & 0x3];
		}
	}

	// As SDL2 does not have texture warping
	// We need to keep 4 copies of the tilemap on the texture side by side
	// so the scroll window can warp around
	std::copy(renderArray, renderArray + (512 * 256), renderArray + (512 * 256));

	source->y = mMap->getRegSCY();
	source->x = mMap->getRegSCX();

	SDL_UpdateTexture(texture, NULL, renderArray, 256 * 4 * 2);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, source, dest);
	SDL_RenderPresent(renderer);
}

void PPU::executePPU(int cycles)
{
	currentClock -= cycles;
	switch (ppuMode)
	{
	case HBLANK:
	{
		if (currentClock < 0)
		{
			Byte LY = mMap->getRegLY();
			mMap->setRegLY(LY + 1);
			if (LY == 0x8F)
			{
				mMap->setRegIF(mMap->getRegIF() | 0x1);
				ppuMode = 1;
			}
			else
			{
				ppuMode = 2;
			}
			currentClock += modeClocks[ppuMode];
		}
	}
		return;
	case VBLANK:
	{
		if (currentClock < 0)
		{
			Byte LY = mMap->getRegLY();
			mMap->setRegLY(LY + 1);
			if (LY == 0x99)
			{
				ppuMode = 2;
				scanlineRendered = false;
				mMap->setRegLY(0);
			}
			currentClock += modeClocks[ppuMode];
		}
	}
		return;
	case OAM:
	{
		if (currentClock < 0)
		{
			// TODO: Implement OAM memory restriction
			ppuMode = 3;
			currentClock += modeClocks[ppuMode];
		}
	}
		return;
	case TRANSFER:
	{
		// TODO: Implement scanline rendering
		if (!scanlineRendered)
		{
			load();
			scanlineRendered = true;
		}

		if (currentClock < 0)
		{
			// TODO: Implement All memory restriction
			ppuMode = 0;
			currentClock += modeClocks[ppuMode];
		}
	}
		return;
	default:
		printf("Unknown PPU Mode %d\n", ppuMode);
		return;
	}
}

void PPU::close()
{
	// Destroy texture
	SDL_DestroyTexture(texture);

	// Destroy renderer
	SDL_DestroyRenderer(renderer);

	// Destroy window
	SDL_DestroyWindow(window);

	// Quit SDL subsystems
	SDL_Quit();
}