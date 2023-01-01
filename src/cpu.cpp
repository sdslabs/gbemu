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
	printf("NOP");
	return 4;
}

int CPU::executeNextInstruction(int opcode)
{
	method_function func = method_pointer[opcode];
	return (this->*func)();
}