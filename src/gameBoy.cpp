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
	// Final State: BC = 0xE102, Flag_N = 0, Flag_H = 0, Flag_Z = 0, AF = 0110
	gbe_mMap->debugWriteMemory(0x0106, 0x04);

	// Set BC = 0XFFFF to test INC B for Z flag
	gbe_mMap->debugWriteMemory(0x0107, 0x01);
	gbe_mMap->debugWriteMemory(0x0108, 0xFF);
	gbe_mMap->debugWriteMemory(0x0109, 0xFF);

	// INC B
	// Increments the value of B by 1
	// Final State: BC = 0x00FF, Flag_N = 0, Flag_H = 1, Flag_Z = 1, AF = 01B0
	gbe_mMap->debugWriteMemory(0x010A, 0x04);

	// Set BC = 0X0FFF to test INC B for H flag
	gbe_mMap->debugWriteMemory(0x010B, 0x01);
	gbe_mMap->debugWriteMemory(0x010C, 0x0F);
	gbe_mMap->debugWriteMemory(0x010D, 0xFF);

	// INC B
	// Increments the value of B by 1
	// Final State: BC = 0x10FF, Flag_N = 0, Flag_H = 1, Flag_Z = 0, AF = 0130
	gbe_mMap->debugWriteMemory(0x010E, 0x04);

	// DEC B
	// Decrements the value of B by 1
	// Final State: BC = 0x09FF, Flag_N = 1, Flag_H = 1, Flag_Z = 0, AF = 0170
	gbe_mMap->debugWriteMemory(0x010F, 0x05);

	// Set BC = 0X01FF to test DEC B for Z flag
	gbe_mMap->debugWriteMemory(0x0110, 0x01);
	gbe_mMap->debugWriteMemory(0x0111, 0x01);
	gbe_mMap->debugWriteMemory(0x0112, 0xFF);

	// DEC B
	// Decrements the value of B by 1
	// Final State: BC = 0x00FF, Flag_N = 1, Flag_H = 0, Flag_Z = 1, AF = 01D0
	gbe_mMap->debugWriteMemory(0x0113, 0x05);

	// Set BC = 0X020F to test DEC B
	gbe_mMap->debugWriteMemory(0x0114, 0x01);
	gbe_mMap->debugWriteMemory(0x0115, 0x02);
	gbe_mMap->debugWriteMemory(0x0116, 0x0F);

	// DEC B
	// Decrements the value of B by 1
	// Final State: BC = 0x010F, Flag_N = 1, Flag_H = 0, Flag_Z = 0, AF = 0150
	gbe_mMap->debugWriteMemory(0x0117, 0x05);

	// LD B, u8
	// Loads an 8 bit immediate into the register B
	// Final State: BC = 0x69FF
	gbe_mMap->debugWriteMemory(0x0118, 0x06);
	gbe_mMap->debugWriteMemory(0x0119, 0x69);

	// RLCA
	// Rotates the value of the accumulator to the left
	// Final State: AF = 0x0200, Flag_C = 0
	gbe_mMap->debugWriteMemory(0x011A, 0x07);

	// Set A = 0x80 to test RLCA for C Flag
	

	// RLCA
	// Will write later when LD A instructions have been tested
	// as that is needed to test this instruction more for flags

	// LD (u16), SP
	// Loads the value of the stack pointer into the memory address pointed to by the immediate
	// Final State: Value 0xFFFE at address 0XE002
	gbe_mMap->debugWriteMemory(0x011B, 0x08);
	gbe_mMap->debugWriteMemory(0x011C, 0xE0);
	gbe_mMap->debugWriteMemory(0x011D, 0x02);

	// ADD HL, BC
	// Adds the value of BC to HL
	// Final State: HL = 0x6BFC
	// This test will fail for now, as the implementation is faulty
	gbe_mMap->debugWriteMemory(0x011E, 0x09);

	// LD BC u16
	// Loading address 0xE003 in BC to test next instruction
	gbe_mMap->debugWriteMemory(0x011F, 0x01);
	gbe_mMap->debugWriteMemory(0x0120, 0xE0);
	gbe_mMap->debugWriteMemory(0x0121, 0x03);

	// LD A, (BC)
	// Loads the value of the memory address pointed to by BC into the accumulator
	// Final State: A = 0xFE, AF = 0xFE00
	gbe_mMap->debugWriteMemory(0x0122, 0x0A);

	// DEC BC
	// Decrements the value of BC by 1
	// Final State: BC = 0xE002
	gbe_mMap->debugWriteMemory(0x0123, 0x0B);

	// INC C
	// Increments the value of C by 1
	// Final State: BC = 0xE003, Flag_N = 0, Flag_H = 0, Flag_Z = 0, AF = 0xFE00
	gbe_mMap->debugWriteMemory(0x0124, 0x0C);

	// LD BC u16
	// Loading address 0xE00F in BC to test H flag of INC C
	gbe_mMap->debugWriteMemory(0x0125, 0x01);
	gbe_mMap->debugWriteMemory(0x0126, 0xE0);
	gbe_mMap->debugWriteMemory(0x0127, 0x0F);

	// INC C
	// Increments the value of C by 1
	// Final State: BC = 0xE010, Flag_N = 0, Flag_H = 1, Flag_Z = 0, AF = 0xFE20
	gbe_mMap->debugWriteMemory(0x0128, 0x0C);
	// Seg fault to end using UNKOWN
	gbe_mMap->debugWriteMemory(0x011B, 0xEB);

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