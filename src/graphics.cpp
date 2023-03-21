#include "types.h"
#include "graphics.h"

PPU::PPU()
{
	// Initialize members
	window = nullptr;
	renderer = nullptr;
	texture = nullptr;
	isEnabled = false;
	event = new SDL_Event();

	// Fill renderArray initially with white (lightest color in palette
	std::fill(renderArray, renderArray + (256 * 256), bg_colors[0]);

	// Copy the same array as a null array (or flush array)
	std::copy(renderArray, renderArray + (256 * 256), nullArray);
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
	if (!(window = SDL_CreateWindow("GameBoy Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 256, SDL_WINDOW_SHOWN)))
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
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 160, 144);

	// Render the texture
	SDL_UpdateTexture(texture, NULL, renderArray, 160 * 4);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	return true;
}

// Poll Events to check for inputs
// And process them
bool PPU::pollEvents()
{
	while (SDL_PollEvent(&(*event)))
	{
		if (event->type == SDL_KEYDOWN)
		{
			printf("Key pressed: %c\n", event->key.keysym.sym);
			if (event->key.keysym.sym == SDLK_ESCAPE)
				exit(0);
		}
	}
	return false;
}

void PPU::load()
{
	// Read background palette register
	bgPalette = mMap->getRegBGP();

	Word tilenum;
	Byte pixelCol;

	// Filling pixel array
	// I am sorry, this is a lot for me to explain
	// I will break it down later
	// Till then, you can attempt to understand this, or give up
	// I suggest you give up
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
				pixelCol = ((*mMap)[bgTileDataAddr + 0x800 + ((SWord)tilenum * 0x10) + (i % 8 * 2)] >> (7 - (j % 8)) & 0x1) + (((*mMap)[bgTileDataAddr + 0x800 + ((SWord)tilenum * 0x10) + (i % 8 * 2) + 1] >> (7 - (j % 8)) & 0x1) * 2);
			}

			renderArray[i * 256 + j] = bg_colors[(bgPalette >> (pixelCol * 2)) & 0x3];
		}
	}

	SDL_UpdateTexture(texture, NULL, renderArray, 256 * 4);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
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