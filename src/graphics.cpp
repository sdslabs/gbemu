#include "types.h"
#include "graphics.h"

PPU::PPU()
{
	// Initialize members
	window = nullptr;
	renderer = nullptr;
	texture = nullptr;
	isEnabled = false;
	showBGWin = false;
	showWindow = false;
	//renderWindow = false;
	mMap = nullptr;
	bgTileDataAddr = 0x0000;
	bgTileMapAddr = 0x0000;
	winTileMapAddr = 0x0000;
	bgPalette = 0x00;
	objPalette0 = 0x00;
	objPalette1 = 0x00;
	currentLine = 0x00;
	hiddenWindowLineCounter = 0x00;
	ppuMode = 0x02;
	event = new SDL_Event();

	ppuMode = 0;
	currentClock = modeClocks[ppuMode];
	scanlineRendered = false;
	frameRendered = false;

	// Fill renderArray initially with white (lightest color in palette)
	std::fill(renderArray, renderArray + (160 * 144), bg_colors[0]);
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
	winTileMapAddr = (LCDC & 0x40) ? 0x9C00 : 0x9800;

	// Evaluate Background Palette register
	bgPalette = mMap->getRegBGP();

	// Evaluate Sprite Palette 0 register
	objPalette0 = mMap->getRegOBP0();

	// Evaluate Sprite Palette 1 register
	objPalette1 = mMap->getRegOBP1();

	// Create a placeholder texture
	// 512x512 to have 4 copies of tilemap
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
	while (SDL_PollEvent(event))
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

void PPU::renderScanline(Byte line)
{
	// Evaluate LCDC register
	Byte LCDC = mMap->getRegLCDC();

	isEnabled = (LCDC & 0x80);
	showBGWin = (LCDC & 0x1);
	showWindow = (LCDC & 0x20);
	showSprites = (LCDC & 0x2);

	if (!isEnabled)
		return;

	bgTileMapAddr = (LCDC & 0x08) ? 0x9C00 : 0x9800;
	bgTileDataAddr = (LCDC & 0x10) ? 0x8000 : 0x8800;
	winTileMapAddr = (LCDC & 0x40) ? 0x9C00 : 0x9800;

	// Read palette registers
	bgPalette = mMap->getRegBGP();
	objPalette0 = mMap->getRegOBP0();
	objPalette1 = mMap->getRegOBP1();

	Byte win_y = mMap->getRegWY();
	Byte win_x = mMap->getRegWX() - 7;
	Byte win_pixel_y = hiddenWindowLineCounter;
	Byte bg_pixel_y = line + mMap->getRegSCY();
	Byte scroll_x = mMap->getRegSCX();
	Byte bg_pixel_x, bg_pixel_col, win_pixel_x, win_pixel_col, sprite_y, sprite_pixel_col;
	Byte bg_tilenum, win_tilenum, sprite_palette;
	Byte sprite_height = (LCDC & 0x4) ? 16 : 8;

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

	for (Byte j = 0; j < 160; j++)
	{
		// Background rendering
		bg_pixel_x = scroll_x + j;
		bg_tilenum = (*mMap)[bgTileMapAddr + ((bg_pixel_y / 8) * 32) + (bg_pixel_x / 8)];

		if (bgTileDataAddr == 0x8800)
		{
			bg_pixel_col = ((*mMap)[bgTileDataAddr + 0x800 + ((SByte)bg_tilenum * 0x10) + (bg_pixel_y % 8 * 2)] >> (7 - (bg_pixel_x % 8)) & 0x1) + ((*mMap)[bgTileDataAddr + 0x800 + ((SByte)bg_tilenum * 0x10) + (bg_pixel_y % 8 * 2) + 1] >> (7 - (bg_pixel_x % 8)) & 0x1) * 2;
		}
		else
		{
			bg_pixel_col = ((*mMap)[bgTileDataAddr + (bg_tilenum * 0x10) + (bg_pixel_y % 8 * 2)] >> (7 - (bg_pixel_x % 8)) & 0x1) + (((*mMap)[bgTileDataAddr + (bg_tilenum * 0x10) + (bg_pixel_y % 8 * 2) + 1] >> (7 - (bg_pixel_x % 8)) & 0x1) * 2);
		}

		if (showBGWin)
			renderArray[(line * 160) + j] = bg_colors[(bgPalette >> (bg_pixel_col * 2)) & 0x3];
		else
			renderArray[(line * 160) + j] = bg_colors[0];

		// Window rendering
		if (showBGWin && showWindow && ((win_y <= line) && (win_y < 144)) && (win_x < 160) && (hiddenWindowLineCounter < 144) && (j >= win_x))
		{
			win_pixel_x = j - win_x;
			win_tilenum = (*mMap)[winTileMapAddr + ((win_pixel_y / 8) * 32) + (win_pixel_x / 8)];

			if (bgTileDataAddr == 0x8800)
			{
				win_pixel_col = ((*mMap)[bgTileDataAddr + 0x800 + ((SByte)win_tilenum * 0x10) + (win_pixel_y % 8 * 2)] >> (7 - (win_pixel_x % 8)) & 0x1) + ((*mMap)[bgTileDataAddr + 0x800 + ((SByte)win_tilenum * 0x10) + (win_pixel_y % 8 * 2) + 1] >> (7 - (win_pixel_x % 8)) & 0x1) * 2;
			}
			else
			{
				win_pixel_col = ((*mMap)[bgTileDataAddr + (win_tilenum * 0x10) + (win_pixel_y % 8 * 2)] >> (7 - (win_pixel_x % 8)) & 0x1) + (((*mMap)[bgTileDataAddr + (win_tilenum * 0x10) + (win_pixel_y % 8 * 2) + 1] >> (7 - (win_pixel_x % 8)) & 0x1) * 2);
			}

			if ((win_pixel_col != 0) || (win_x))
				renderArray[(line * 160) + j] = bg_colors[(bgPalette >> (win_pixel_col * 2)) & 0x3];
		}
	}

	if (showBGWin && showWindow && ((win_y <= line) && (win_y < 144)) && (win_x < 160) && (hiddenWindowLineCounter < 144))
		hiddenWindowLineCounter++;

	// Sprite rendering
	if (showSprites)
	{
		sprites.clear();
		for (Word i = 0xFE00; i < 0xFEA0; i += 4)
		{
			if (sprites.size() >= 10)
				break;
			sprite_y = (*mMap)[i];
			if ((line < (sprite_y - 16) || line > (sprite_y - 16 + sprite_height)))
				continue;

			Sprite* sprite = new Sprite();
			sprite->address = i;
			sprite->y = sprite_y;
			sprite->x = (*mMap)[i + 1];
			sprite->tile = (*mMap)[i + 2];
			sprite->flags = (*mMap)[i + 3];
			sprites.push_back(*sprite);
		}

		if (sprites.size())
			std::sort(sprites.begin(), sprites.end(), [](Sprite& a, Sprite& b) { return (((a.x == b.x) && (a.address > b.address)) || (a.x > b.x)); });
		
		for (auto it = sprites.begin(); it != sprites.end(); ++it)
		{
			sprite_palette = (it->flags & 0x10) ? objPalette1 : objPalette0;
			for (int i = 0; i < 8; i++)
			{
				if (sprite_height == 16)
					it->tile &= 0xFE;
				switch (it->flags & 0x60)
				{
					case 0x00: // Normal
						sprite_pixel_col = ((*mMap)[0x8000 + (it->tile * 0x10) + ((line - (it->y - 16)) * 2)] >> (7 - i) & 0x1) + (((*mMap)[0x8000 + (it->tile * 0x10) + ((line - (it->y - 16)) * 2) + 1] >> (7 - i) & 0x1) * 2);
						break;
					case 0x20: // Flip X
						sprite_pixel_col = ((*mMap)[0x8000 + (it->tile * 0x10) + ((line - (it->y - 16)) * 2)] >> i & 0x1) + (((*mMap)[0x8000 + (it->tile * 0x10) + ((line - (it->y - 16)) * 2) + 1] >> i & 0x1) * 2);
						break;
					case 0x40: // Flip Y
						sprite_pixel_col = ((*mMap)[0x8000 + (it->tile * 0x10) + ((sprite_height - (line - (it->y - 16)) - 1) * 2)] >> (7 - i) & 0x1) + (((*mMap)[0x8000 + (it->tile * 0x10) + ((sprite_height - (line - (it->y - 16)) - 1) * 2) + 1] >> (7 - i) & 0x1) * 2);
						break;
					case 0x60: // Flip X and Y
						sprite_pixel_col = ((*mMap)[0x8000 + (it->tile * 0x10) + ((sprite_height - (line - (it->y - 16)) - 1) * 2)] >> i & 0x1) + (((*mMap)[0x8000 + (it->tile * 0x10) + ((sprite_height - (line - (it->y - 16)) - 1) * 2) + 1] >> i & 0x1) * 2);
						break;
					default:
						break;
				}

				if (sprite_pixel_col != 0)
				{
					if (((it->x + i - 8) < 160) && !(it->flags & 0x80))
						renderArray[(line * 160) + (it->x + i - 8)] = bg_colors[(sprite_palette >> (sprite_pixel_col * 2)) & 0x3];
				}
			}
		}
	}
}

void PPU::executePPU(int cycles)
{
	currentClock -= cycles;
	switch (ppuMode)
	{
	case HBLANK:
	{
		if (!scanlineRendered)
		{
			renderScanline(mMap->getRegLY());
			scanlineRendered = true;
		}

		if (currentClock < 0)
		{
			Byte LY = mMap->getRegLY();
			Byte LYC = mMap->getRegLYC();
			Byte STAT = mMap->getRegSTAT();
			mMap->setRegLY(LY + 1);

			if (LY + 1 == LYC)
			{
				mMap->setRegSTAT(STAT | 0x4);
				if (STAT & 0x40)
					mMap->setRegIF(mMap->getRegIF() | 0x2);
			}
			else
			{
				mMap->setRegSTAT(STAT & 0xFB);
			}

			if (LY == 0x8F)
			{
				mMap->setRegIF(mMap->getRegIF() | 0x1);
				mMap->setRegSTAT((STAT & 0xFC) | 0x1);
				if (STAT & 0x10)
					mMap->setRegIF(mMap->getRegIF() | 0x2);
				ppuMode = 1;
				hiddenWindowLineCounter = 0;
			}
			else
			{
				mMap->setRegSTAT((STAT & 0xFC) | 0x2);
				if (STAT & 0x20)
					mMap->setRegIF(mMap->getRegIF() | 0x2);
				ppuMode = 2;
			}
			currentClock += modeClocks[ppuMode];
		}
	}
	break;
	case VBLANK:
	{
		if (!frameRendered)
		{
			SDL_UpdateTexture(texture, NULL, renderArray, 160 * 4);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
			frameRendered = true;
		}
		if (currentClock < 0)
		{
			Byte LY = mMap->getRegLY();
			Byte LYC = mMap->getRegLYC();
			Byte STAT = mMap->getRegSTAT();
			mMap->setRegLY(LY + 1);

			if (LYC == LY + 1)
			{
				mMap->setRegSTAT(STAT & 0x4);
				if (STAT & 0x40)
					mMap->setRegIF(mMap->getRegIF() | 0x2);
			}
			else
			{
				mMap->setRegSTAT(STAT & 0xFB);
			}

			if (LY == 0x99)
			{
				mMap->setRegSTAT((STAT & 0xFC) | 0x2);
				if (STAT & 0x20)
					mMap->setRegIF(mMap->getRegIF() | 0x2);
				ppuMode = 2;
				scanlineRendered = false;
				mMap->setRegLY(0);
				if (LYC == 0)
				{
					mMap->setRegSTAT(STAT & 0x4);
					if (STAT & 0x40)
						mMap->setRegIF(mMap->getRegIF() | 0x2);
				}
				else
				{
					mMap->setRegSTAT(STAT & 0xFB);
				}
			}
			currentClock += modeClocks[ppuMode];
		}
	}
	break;
	case OAM:
	{
		frameRendered = false;
		if (currentClock < 0)
		{
			// TODO: Implement OAM memory restriction
			Byte STAT = mMap->getRegSTAT();
			mMap->setRegSTAT((STAT & 0xFC) | 0x3);
			ppuMode = 3;
			currentClock += modeClocks[ppuMode];
		}
	}
	break;
	case TRANSFER:
	{
		scanlineRendered = false;

		if (currentClock < 0)
		{
			// TODO: Implement All memory restriction
			Byte STAT = mMap->getRegSTAT();
			mMap->setRegSTAT(STAT & 0xFC);
			if (STAT & 0x8)
				mMap->setRegIF(mMap->getRegIF() | 0x2);
			ppuMode = 0;
			currentClock += modeClocks[ppuMode];
		}
	}
	break;
	default:
		printf("Unknown PPU Mode %d\n", ppuMode);
		break;
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