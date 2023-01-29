#include "ppu.h"

PPU::PPU()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	m_Window = SDL_CreateWindow("GameBoy Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160, 144, SDL_WINDOW_SHOWN);
	m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED);
}

void PPU::render()
{

}

// Get VRAM from MemoryMap and render it to the screen
void PPU::UpdateGraphics()
{
	if (SDL_PollEvent( & m_Event)) {
		if (m_Event.type == SDL_QUIT) {
			m_IsRunning = false;
		}
	}
	SDL_SetRenderDrawColor( m_Renderer, 0xFF, 0xFF, 0xFF, 0 ); // line of code in question

	SDL_Rect fillRect = { 0, 0, 20, 20 };
	SDL_SetRenderDrawColor( m_Renderer, 0x00, 0xFF, 0x00, 0xFF ); // 2nd line of code in question
	SDL_RenderFillRect( m_Renderer, &fillRect );

	SDL_RenderPresent(m_Renderer);
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

