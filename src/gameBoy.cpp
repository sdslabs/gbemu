#include "types.h"
#include "cpu.h"
#include "gameBoy.h"

GBE::GBE()
{
	// Initialize the CPU
	gbe_cpu = new CPU();

	// Initialize the MemoryMap
	gbe_mMap = new MemoryMap();

	// Unify the CPU and MemoryMap
	gbe_cpu->setMemory(gbe_mMap);

	gbe_mMap->debugWriteMemory(0x0100, 0x00);

	// Dec B
	gbe_mMap->debugWriteMemory(0x0102, 0x05);

	//RLCA
	gbe_mMap->debugWriteMemory(0x0103, 0x07);

	update();
}

void GBE::update()
{
	// Update function of the GBE
	// Will be called every frame
	// GB has 59.73 frames per second

	int cycles = 0;
	while (cycles < gbe_cpu->clockSpeedPerFrame)
	{
		// Execute the next instruction
		cycles += gbe_cpu->executeNextInstruction();
		// updateTimers()
		// updateGraphics()
		// Do Interrupts()
	}
	// renderGraphics()
}