#pragma once
#include "types.h"

// CPU Register
// Pulled from https://gbdev.io/pandocs/CPU_Registers_and_Flags.html
// 1 word register
// lo and hi bytes are reversed for little endian

union Register
{
	Word dat;
	struct
	{
		Byte lo;
		Byte hi;
	};
};

// CPU
// Pulled from https://gbdev.io/pandocs/CPU_Registers_and_Flags.html
// Contains all the registers and flags
class CPU
{

private:
	// Accumulator and Flags
	Register reg_AF;

	// General Purpose Registers
	Register reg_BC;
	Register reg_DE;
	Register reg_HL;

	// Stack Pointer
	Register reg_SP;

	// Program Counter
	Register reg_PC;
};