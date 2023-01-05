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

	// Current State: AF = 0x01B0, BC = 0x0013, DE = 0x00D8, HL = 0x014D, SP = 0xFFFE, PC = 0x0100

	// NOP
	// Does nothing
	// Final State: No change
	gbe_mMap->debugWriteMemory(0x0100, 0x00);

	// LD BC, u16
	// Loads a 16 bit immediate into the register BC
	// Final State: BC = 0XE001
	gbe_mMap->debugWriteMemory(0x0101, 0x01);
	gbe_mMap->debugWriteMemory(0x0102, 0xE0);
	gbe_mMap->debugWriteMemory(0x0103, 0x01);

	// LD (BC), A
	// Loads the value of the accumulator into the memory address pointed to by BC
	// Final State: Value 0x01 at address 0xE001 (2nd byte of Work RAM)
	gbe_mMap->debugWriteMemory(0x0104, 0x02);

	// INC BC
	// Increments the value of BC by 1
	// Final State: BC = 0xE002
	gbe_mMap->debugWriteMemory(0x0105, 0x03);

	// INC B
	// Increments the value of B by 1
	// Final State: BC = 0xE102, Flag_N = 0, Flag_H = 0, Flag_Z = 0
	gbe_mMap->debugWriteMemory(0x0106, 0x04);

	// Set BC = 0XFFFF to test INC B for Z flag
	gbe_cpu->set_reg_BC(0xFFFF);

	// INC B
	// Increments the value of B by 1
	// Final State: BC = 0x00FF, Flag_N = 0, Flag_H = 0, Flag_Z = 1
	gbe_mMap->debugWriteMemory(0x0107, 0x04);

	// Set BC = 0X0FFF to test INC B for H flag
	gbe_cpu->set_reg_BC(0x0FFF);

	// INC B
	// Increments the value of B by 1
	// Final State: BC = 0x10FF, Flag_N = 0, Flag_H = 1, Flag_Z = 0
	gbe_mMap->debugWriteMemory(0x0108, 0x04);

	// DEC B
	// Decrements the value of B by 1
	// Final State: BC = 0x09FF, Flag_N = 1, Flag_H = 1, Flag_Z = 0
	gbe_mMap->debugWriteMemory(0x0109, 0x05);

	// Set BC = 0X01FF to test DEC B for Z flag
	gbe_cpu->set_reg_BC(0x01FF);

	// DEC B
	// Decrements the value of B by 1
	// Final State: BC = 0x00FF, Flag_N = 1, Flag_H = 0, Flag_Z = 1
	gbe_mMap->debugWriteMemory(0x010A, 0x05);

	// Set BC = 0X020F to test DEC B
	gbe_cpu->set_reg_BC(0x020F);

	// DEC B
	// Decrements the value of B by 1
	// Final State: BC = 0x010F, Flag_N = 1, Flag_H = 0, Flag_Z = 0
	gbe_mMap->debugWriteMemory(0x010B, 0x05);

	// LD B, u8
	// Loads an 8 bit immediate into the register B
	// Final State: BC = 0x69FF
	gbe_mMap->debugWriteMemory(0x010C, 0x06);
	gbe_mMap->debugWriteMemory(0x010D, 0x69);

	// RLCA
	// Rotates the value of the accumulator to the left
	// Final State: AF = 0x8040, Flag_C = 0
	gbe_mMap->debugWriteMemory(0x010E, 0x07);

	// Set A = 0x80 to test RLCA for C Flag

	// //RLCA
	// gbe_mMap->debugWriteMemory(0x0103, 0x07);

	// // LD (BC), A
	// gbe_mMap->debugWriteMemory(0x0104, 0x02);

	// // LD (u16), A
	// gbe_mMap->debugWriteMemory(0x0105, 0xEA);
	// gbe_mMap->debugWriteMemory(0x0106, 0x69);
	// gbe_mMap->debugWriteMemory(0x0107, 0x69);

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