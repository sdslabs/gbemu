#include "types.h"
#include "cpu.h"
#include <stdio.h>

CPU::CPU()
{

	// The GameBoy Power Up Sequence
	// Pulled from https://gbdev.io/pandocs/Power_Up_Sequence.html#cpu-registers
	// We are following the DMG boot ROM

	// Set the Program Counter to 0x0100
	reg_PC.dat = 0x0100;

	// Set Accumulator to 0x01 and Flags to Z = 1, N = 0, H = 1, C = 1
	// Assuming header checksum passes
	reg_AF.dat = 0x01B0;

	reg_BC.dat = 0x0013;
	reg_DE.dat = 0x00D8;
	reg_HL.dat = 0x014D;
	reg_SP.dat = 0xFFFE;
}

// NOP just adds 4 cycles
// Does nothing
int CPU::NOP()
{
	reg_PC.dat += 1;
	printf("NOP");
	return 4;
}

// LD BC, u16
// Loads a 16 bit immediate value into BC
int CPU::LD_BC_u16()
{
	// Load the next 2 bytes into BC
	// Left shift the first byte by 8 bits
	// OR the second byte
	reg_BC.dat = ((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2];
	reg_PC.dat += 3;
	printf("LD BC, u16");
	return 12;
}

int CPU::executeNextInstruction()
{
	// Get the opcode
	Byte opcode = (*mMap)[reg_PC.dat];	
	return (this->*method_pointer[opcode])();
}