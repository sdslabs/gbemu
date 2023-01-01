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

    // Flags
    // Pulled from https://gbdev.io/pandocs/CPU_Registers_and_Flags.html
    // Naming convention is: FLAG_<name>_<bit>
    // Bit 0-3 are not used
    enum Flags
    {
        FLAG_CARRY_c = 4,
        FLAG_HALF_CARRY_h = 5,
        FLAG_SUBTRACT_n = 6,
        FLAG_ZERO_z = 7
    };

    // Complete Address Space
    // Where CPU can write
    // 16 bit address space
    // Pulled from https://gbdev.io/pandocs/Memory_Map.html
    Byte addressSpace[0x10000];

public:
    const int clockSpeed = 4194304; // 4.194304 MHz CPU
    const int clockSpeedPerFrame = 70224; // 4194304 / 59.73fps

    
    CPU();
    int executeNextInstruction();
};