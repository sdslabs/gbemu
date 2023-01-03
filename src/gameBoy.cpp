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
	// Test
	// zero flag should be set
	gbe_mMap->debugWriteMemory(0x0101, 0x01);
	gbe_mMap->debugWriteMemory(0x0102, 0x01);
	gbe_mMap->debugWriteMemory(0x0103, 0x00);
	gbe_mMap->debugWriteMemory(0x0104, 0x05);
	// half carry flag should be set
	gbe_mMap->debugWriteMemory(0x0105, 0x01);
	gbe_mMap->debugWriteMemory(0x0106, 0x10);
	gbe_mMap->debugWriteMemory(0x0107, 0x00);
	gbe_mMap->debugWriteMemory(0x0108, 0x05);



//	//RLCA
//	gbe_mMap->debugWriteMemory(0x0103, 0x07);
//
//	// LD (BC), A
//	gbe_mMap->debugWriteMemory(0x0104, 0x02);
//
//	// LD (u16), A
//	gbe_mMap->debugWriteMemory(0x0105, 0xEA);
//	gbe_mMap->debugWriteMemory(0x0106, 0x69);
//	gbe_mMap->debugWriteMemory(0x0107, 0x69);

	// Seg fault to end using UNKOWN
	gbe_mMap->debugWriteMemory(0x0109, 0xEB);

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