#include "types.h"
#include "cpu.h"
#include "gameBoy.h"

int GBE::s_Cycles;

GBE::GBE()
{
	// Initialize the CPU
	gbe_cpu = new CPU();

	// Initialize the MemoryMap
	gbe_mMap = new MemoryMap();

	// Initialize the Graphics
	gbe_graphics = new PPU();

	// debug mode = false
	debug_mode = false;

	// Init event
	event = new SDL_Event();

	// Unify the CPU and MemoryMap
	gbe_cpu->setMemory(gbe_mMap);

	// Unify the CPU and PPU
	gbe_cpu->setPPU(gbe_graphics);

	// Unify the PPU and MmeoryMap
	gbe_graphics->setMemoryMap(gbe_mMap);

	gbe_graphics->init();

	// Open the Boot ROM
	if ((bootROM = fopen("../src/dmg_boot.gb", "rb")) == NULL)
		printf("boot rom file not opened");

	// Open the Game ROM
	if ((gameROM = fopen("../tests/halt_bug.gb", "rb")) == NULL)
		printf("game rom file not opened");

	// Set the Boot ROM
	gbe_mMap->setBootRomFile(bootROM);

	// Set the Game ROM
	gbe_mMap->setRomFile(gameROM);

	// Map to ROMs to gbe_mMap
	gbe_mMap->mapRom();

	s_Cycles = 0;

	// Adding the Nintendo Logo to ROM
	// to pass the Boot check

	gbe_mMap->debugWriteMemory(0x104, 0xCE);
	gbe_mMap->debugWriteMemory(0x105, 0xED);
	gbe_mMap->debugWriteMemory(0x106, 0x66);
	gbe_mMap->debugWriteMemory(0x107, 0x66);
	gbe_mMap->debugWriteMemory(0x108, 0xCC);
	gbe_mMap->debugWriteMemory(0x109, 0x0D);
	gbe_mMap->debugWriteMemory(0x10A, 0x00);
	gbe_mMap->debugWriteMemory(0x10B, 0x0B);
	gbe_mMap->debugWriteMemory(0x10C, 0x03);
	gbe_mMap->debugWriteMemory(0x10D, 0x73);
	gbe_mMap->debugWriteMemory(0x10E, 0x00);
	gbe_mMap->debugWriteMemory(0x10F, 0x83);
	gbe_mMap->debugWriteMemory(0x110, 0x00);
	gbe_mMap->debugWriteMemory(0x111, 0x0C);
	gbe_mMap->debugWriteMemory(0x112, 0x00);
	gbe_mMap->debugWriteMemory(0x113, 0x0D);
	gbe_mMap->debugWriteMemory(0x114, 0x00);
	gbe_mMap->debugWriteMemory(0x115, 0x08);
	gbe_mMap->debugWriteMemory(0x116, 0x11);
	gbe_mMap->debugWriteMemory(0x117, 0x1F);
	gbe_mMap->debugWriteMemory(0x118, 0x88);
	gbe_mMap->debugWriteMemory(0x119, 0x89);
	gbe_mMap->debugWriteMemory(0x11A, 0x00);
	gbe_mMap->debugWriteMemory(0x11B, 0x0E);
	gbe_mMap->debugWriteMemory(0x11C, 0xDC);
	gbe_mMap->debugWriteMemory(0x11D, 0xCC);
	gbe_mMap->debugWriteMemory(0x11E, 0x6E);
	gbe_mMap->debugWriteMemory(0x11F, 0xE6);
	gbe_mMap->debugWriteMemory(0x120, 0xDD);
	gbe_mMap->debugWriteMemory(0x121, 0xDD);
	gbe_mMap->debugWriteMemory(0x122, 0xD9);
	gbe_mMap->debugWriteMemory(0x123, 0x99);
	gbe_mMap->debugWriteMemory(0x124, 0xBB);
	gbe_mMap->debugWriteMemory(0x125, 0xBB);
	gbe_mMap->debugWriteMemory(0x126, 0x67);
	gbe_mMap->debugWriteMemory(0x127, 0x63);
	gbe_mMap->debugWriteMemory(0x128, 0x6E);
	gbe_mMap->debugWriteMemory(0x129, 0x0E);
	gbe_mMap->debugWriteMemory(0x12A, 0xEC);
	gbe_mMap->debugWriteMemory(0x12B, 0xCC);
	gbe_mMap->debugWriteMemory(0x12C, 0xDD);
	gbe_mMap->debugWriteMemory(0x12D, 0xDC);
	gbe_mMap->debugWriteMemory(0x12E, 0x99);
	gbe_mMap->debugWriteMemory(0x12F, 0x9F);
	gbe_mMap->debugWriteMemory(0x130, 0xBB);
	gbe_mMap->debugWriteMemory(0x131, 0xB9);
	gbe_mMap->debugWriteMemory(0x132, 0x33);
	gbe_mMap->debugWriteMemory(0x133, 0x3E);

	executeBootROM();

	update();
}

void GBE::update()
{
	// Update function of the GBE
	// Will be called every frame
	// GB has 59.73 frames per second
	while (true)
	{
		// Execute the next instruction
		s_Cycles += gbe_cpu->executeNextInstruction();

		// update the DIV and TIMA timers
		gbe_cpu->updateTimers(s_Cycles);
		gbe_graphics->executePPU(s_Cycles);
		s_Cycles = 0;
		s_Cycles += gbe_cpu->performInterrupt();
		pollEvents();
		if (debug_mode)
		{
			debug_int();
		}
	}
}

void GBE::getValueOfRegister(char registerName)
{
	printf("Register: %c\n", registerName);
	Word value;

	switch (registerName)
	{
	case 'A':
		value = gbe_cpu->get_reg_A();
		break;
	case 'B':
		value = gbe_cpu->get_reg_B();
		break;
	case 'C':
		value = gbe_cpu->get_reg_C();
		break;
	case 'D':
		value = gbe_cpu->get_reg_D();
		break;
	case 'E':
		value = gbe_cpu->get_reg_E();
		break;
	case 'H':
		value = gbe_cpu->get_reg_H();
		break;
	case 'L':
		value = gbe_cpu->get_reg_L();
		break;
	default:
		break;
	}
	// printf(value);
	printf("%hu\n", value);
}

void GBE::debug_int()
{
	printf("\nEntered debug mode\n");
	bool shouldAdv = false;
	bool infoMode;
	bool debuggerMode = false;
	while (!shouldAdv)
	{
		while (SDL_PollEvent(event))
		{
			if (event->key.type == SDL_KEYUP)
			{
				switch (event->key.keysym.sym)
				{
				case SDLK_s:
					printf("s pressed\n");
					shouldAdv = true;
					break;
				case SDLK_u:
					debug_mode = !debug_mode;
					printf("pressed d\n");
					shouldAdv = true;
					break;
				// Left control to open Debugger window
				// b for bgMap
				// t to show current tiles on screen
				// s to show sprites on screen
				case SDLK_LCTRL:
					gbe_graphics->debuggerInit();
					// gbe_graphics->render_ttl();
					debuggerMode = true;
					while (debuggerMode)
					{
						while (SDL_PollEvent(event))
						{
							if (event->key.type == SDL_KEYUP)
							{
								switch (event->key.keysym.sym)
								{
								case SDLK_b:
									gbe_graphics->listBgMap();
									break;
								case SDLK_s:
									gbe_graphics->renderOAM();
									break;
								case SDLK_t:
									gbe_graphics->listTiles();
									break;
								case SDLK_ESCAPE:
									gbe_graphics->close(true);
									debuggerMode = false;
									break;
								default:
									break;
								}
							}
						}
					}
					break;
				case SDLK_i:
					printf("Info Mode:- \n");
					infoMode = true;
					shouldAdv = true;
					// Press r to print all registers
					// enter a for A
					// enter d for D
					// enter c for C
					// enter b for B
					// enter e for E
					// enter h for H
					// enter l for L
					// press left ctrl to open V-Ram debugger window
					// enter p to print the stack
					// enter w to write to memory
					// enter x to exit info mode
					while (infoMode)
					{
						while (SDL_PollEvent(event))
						{
							if (event->key.type == SDL_KEYUP)
							{
								switch (event->key.keysym.sym)
								{
								case SDLK_r:
									getValueOfRegister('A');
									getValueOfRegister('B');
									getValueOfRegister('C');
									getValueOfRegister('D');
									getValueOfRegister('E');
									getValueOfRegister('H');
									getValueOfRegister('L');
									break;
								case SDLK_a:
									getValueOfRegister('A');
									break;
								case SDLK_b:
									getValueOfRegister('B');
									break;
								case SDLK_c:
									getValueOfRegister('C');
									break;
								case SDLK_d:
									getValueOfRegister('D');
									break;
								case SDLK_e:
									getValueOfRegister('E');
									break;
								case SDLK_h:
									getValueOfRegister('H');
									break;
								case SDLK_l:
									getValueOfRegister('L');
									break;
								case SDLK_p:
									gbe_cpu->printStack();
									break;
								case SDLK_w:
									gbe_cpu->writeToMemory();
									break;
								case SDLK_x:
									infoMode = false;
									break;
								default:
									break;
								}
							}
						}
					}
					break;
				default:
					break;
				}
			}
		}
	}
}

void GBE::executeBootROM()
{
	while (gbe_mMap->readMemory(0xFF50) == 0x00)
	{
		s_Cycles += gbe_cpu->executeNextInstruction();
		gbe_cpu->updateTimers(s_Cycles);
		gbe_graphics->executePPU(s_Cycles);
		s_Cycles = 0;
		s_Cycles += gbe_cpu->performInterrupt();
	}

	gbe_mMap->unloadBootRom();
}

// Poll Events to check for inputs
// And process them
bool GBE::pollEvents()
{
	while (SDL_PollEvent(event))
	{
		if (event->key.type == SDL_KEYDOWN)
		{
			switch (event->key.keysym.sym)
			{
			case SDLK_LEFT:
				*(gbe_mMap->joyPadState) &= 0xFD;
				break;
			case SDLK_RIGHT:
				*(gbe_mMap->joyPadState) &= 0xFE;
				break;
			case SDLK_UP:
				*(gbe_mMap->joyPadState) &= 0xFB;
				break;
			case SDLK_DOWN:
				*(gbe_mMap->joyPadState) &= 0xF7;
				break;
			case SDLK_a:
				*(gbe_mMap->joyPadState) &= 0xEF;
				break;
			case SDLK_s:
				*(gbe_mMap->joyPadState) &= 0xDF;
				break;
			case SDLK_LSHIFT:
				*(gbe_mMap->joyPadState) &= 0xBF;
				break;
			case SDLK_SPACE:
				*(gbe_mMap->joyPadState) &= 0x7F;
				break;
			case SDLK_d:
				debug_mode = !debug_mode;
				break;
			case SDLK_ESCAPE:
				gbe_graphics->close(false);
				exit(0);
			default:
				break;
			}
		}
		else if (event->key.type == SDL_KEYUP)
		{
			switch (event->key.keysym.sym)
			{
			case SDLK_LEFT:
				*(gbe_mMap->joyPadState) |= 0x02;
				break;
			case SDLK_RIGHT:
				*(gbe_mMap->joyPadState) |= 0x01;
				break;
			case SDLK_UP:
				*(gbe_mMap->joyPadState) |= 0x04;
				break;
			case SDLK_DOWN:
				*(gbe_mMap->joyPadState) |= 0x08;
				break;
			case SDLK_a:
				*(gbe_mMap->joyPadState) |= 0x10;
				break;
			case SDLK_s:
				*(gbe_mMap->joyPadState) |= 0x20;
				break;
			case SDLK_LSHIFT:
				*(gbe_mMap->joyPadState) |= 0x40;
				break;
			case SDLK_SPACE:
				*(gbe_mMap->joyPadState) |= 0x80;
				break;
			default:
				break;
			}
		}
	}
	return false;
}