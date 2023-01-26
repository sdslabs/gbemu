#include "types.h"
#include "cpu.h"
#include "gameBoy.h"

int GBE::s_Cycles;

GBE::GBE()
{
	// Initialize the CPU
	gbe_cpu = MakeRef<CPU>();

	// Initialize the PPU
	gbe_ppu = MakeRef<PPU>();

	// Initialize the MemoryMap
	gbe_mMap = MakeRef<MemoryMap>();

	// Unify the CPU and MemoryMap
	gbe_cpu->setMemory(*gbe_mMap.get());

	// Unify the PPU and MemoryMap
	gbe_ppu->SetMemory(*gbe_mMap.get());

	// Open the Boot ROM
	if ((bootROM = fopen("../src/dmg_boot.gb", "rb")) == NULL)
		printf("boot rom file not opened");

	// Open the Game ROM
	if ((gameROM = fopen("../tests/cpu_instrs/individual/02-interrupts.gb", "rb")) == NULL)
		printf("game rom file not opened");

	// Load the Boot ROM
	// Into the first 0x100 bytes
	fread(gbe_mMap->getRomBank0(), 1, 256, bootROM);

	// Load Game ROM in Bank 0
	// After offsetting for Boot ROM first
	fseek(gameROM, 0x100, SEEK_SET);

	fread(gbe_mMap->getRomBank0() + 0x100, 1, 16128, gameROM);
	fread(gbe_mMap->getRomBank1(), 1, 16384, gameROM);

	s_Cycles = 0;

	// STUB: Fooling the emulator
	// Into thinking the frame is ready
	// Helps us get out of the loop at
	// 0x0064 in the boot ROM
	// Needs to be removed once Timers
	// and PPU is implemented
	gbe_mMap->writeMemory(0xff44, 0x90);

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

	// Using the Tetris header to pass the checksum test
	// Unnecessary if loading a valid rom

	/*gbe_mMap->debugWriteMemory(0x134, 0x54);
	gbe_mMap->debugWriteMemory(0x135, 0x45);
	gbe_mMap->debugWriteMemory(0x136, 0x54);
	gbe_mMap->debugWriteMemory(0x137, 0x52);
	gbe_mMap->debugWriteMemory(0x138, 0x49);
	gbe_mMap->debugWriteMemory(0x139, 0x53);
	gbe_mMap->debugWriteMemory(0x13A, 0x00);
	gbe_mMap->debugWriteMemory(0x13B, 0x00);
	gbe_mMap->debugWriteMemory(0x13C, 0x00);
	gbe_mMap->debugWriteMemory(0x13D, 0x00);
	gbe_mMap->debugWriteMemory(0x13E, 0x00);
	gbe_mMap->debugWriteMemory(0x13F, 0x00);
	gbe_mMap->debugWriteMemory(0x140, 0x00);
	gbe_mMap->debugWriteMemory(0x141, 0x00);
	gbe_mMap->debugWriteMemory(0x142, 0x00);
	gbe_mMap->debugWriteMemory(0x143, 0x00);
	gbe_mMap->debugWriteMemory(0x144, 0x00);
	gbe_mMap->debugWriteMemory(0x145, 0x00);
	gbe_mMap->debugWriteMemory(0x146, 0x00);
	gbe_mMap->debugWriteMemory(0x147, 0x00);
	gbe_mMap->debugWriteMemory(0x148, 0x00);
	gbe_mMap->debugWriteMemory(0x149, 0x00);
	gbe_mMap->debugWriteMemory(0x14A, 0x00);
	gbe_mMap->debugWriteMemory(0x14B, 0x01);
	gbe_mMap->debugWriteMemory(0x14C, 0x01);
	gbe_mMap->debugWriteMemory(0x14D, 0x0A);*/

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
		if ((*gbe_mMap)[0xFF02] == 0x81)
		{
			printf("%c", (*gbe_mMap)[0xFF01]);
			gbe_mMap->writeMemory(0xFF02, 0x00);
		}

		// update the DIV and TIMA timers
		gbe_cpu->updateTimers(s_Cycles);
		//		gbe_ppu->UpdateGraphics();
		s_Cycles = 0;
		s_Cycles += gbe_cpu->performInterrupt();
	}
}

void GBE::executeBootROM()
{
	while (gbe_mMap->readMemory(0xFF50) == 0x00)
	{
		s_Cycles += gbe_cpu->executeNextInstruction();
	}

	// Overwrite the boot ROM with first 256 bytes of game ROM
	fseek(gameROM, 0x00, SEEK_SET);
	fread(gbe_mMap->getRomBank0(), 1, 256, gameROM);
}