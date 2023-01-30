#include "types.h"
#include "cpu.h"
#include <stdio.h>
#ifndef DEBUG
#define debugPrint(...)
#endif
#ifdef DEBUG
#define debugPrint(...) printf(__VA_ARGS__)
#endif

// Setters of Flags
// Flags are stored in the higher 4 bits of the F register
// Flags are set by bitwise OR
#define SET_ZERO_FLAG reg_AF.lo |= FLAG_ZERO_z
#define SET_SUBTRACT_FLAG reg_AF.lo |= FLAG_SUBTRACT_n
#define SET_HALF_CARRY_FLAG reg_AF.lo |= FLAG_HALF_CARRY_h
#define SET_CARRY_FLAG reg_AF.lo |= FLAG_CARRY_c

// Unsetters of Flags
// Flags are stored in the higher 4 bits of the F register
// Flags are unset by bitwise AND
#define UNSET_ZERO_FLAG reg_AF.lo &= ~FLAG_ZERO_z
#define UNSET_SUBTRACT_FLAG reg_AF.lo &= ~FLAG_SUBTRACT_n
#define UNSET_HALF_CARRY_FLAG reg_AF.lo &= ~FLAG_HALF_CARRY_h
#define UNSET_CARRY_FLAG reg_AF.lo &= ~FLAG_CARRY_c

// Getters of Flags
// Flags are stored in the higher 4 bits of the F register
// Flags are retrieved by bitwise AND
// Flags are shifted to the right by 7, 6, 5, and 4 bits respectively
#define GET_ZERO_FLAG ((reg_AF.lo & FLAG_ZERO_z) >> 7)
#define GET_SUBTRACT_FLAG ((reg_AF.lo & FLAG_SUBTRACT_n) >> 6)
#define GET_HALF_CARRY_FLAG ((reg_AF.lo & FLAG_HALF_CARRY_h) >> 5)
#define GET_CARRY_FLAG ((reg_AF.lo & FLAG_CARRY_c) >> 4)

CPU::CPU()
{

	// The GameBoy Power Up Sequence
	// Pulled from https://gbdev.io/pandocs/Power_Up_Sequence.html#cpu-registers
	// We are following the DMG boot ROM

	// Set the Program Counter to 0x0000
	reg_PC.dat = 0x0000;
	reg_AF.dat = 0x0000;
	reg_BC.dat = 0x0000;
	reg_DE.dat = 0x0000;
	reg_HL.dat = 0x0000;
	reg_SP.dat = 0x0000;

	// set the timer_counters
	timer_counter.div = 0;
	timer_counter.tima = 0;

	// Set isLowPower to false
	isLowPower = false;

	// Set isHalted to false
	isHalted = false;

	// The debug logging file
	outfile = fopen("logfile.txt", "w");

	// TODO: check the initial state of IME
	IMEFlag = -1;

	IMEReg = false;
}

// NOP just adds 4 cycles
// Does nothing
int CPU::NOP()
{
	reg_PC.dat += 1;
	debugPrint("NOP\n");
	return 4;
}

// LD BC, u16
// Loads a 16 bit immediate value into BC
int CPU::LD_BC_u16()
{
	// Load the next 2 bytes into BC
	// Left shift the second byte by 8 bits
	// OR the first byte
	// Due to endianness, the first byte is the least significant byte
	reg_BC.dat = ((*mMap)[reg_PC.dat + 2] << 8) | (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 3;
	debugPrint("LD BC, u16\n");
	return 12;
}

// LD (BC), A
// Loads the contents of A into the memory address pointed to by BC
int CPU::LD_BC_A()
{
	// Write the contents of A into the memory address pointed to by BC
	mMap->writeMemory(reg_BC.dat, reg_AF.hi);

	// Increment the program counter
	reg_PC.dat += 1;

	debugPrint("LD (BC), A\n");
	return 8;
}

// INC BC
// Increments the contents of BC
int CPU::INC_BC()
{
	reg_BC.dat += 1;
	reg_PC.dat += 1;
	debugPrint("INC BC\n");
	return 8;
}

// INC B
// Increments the contents of B
int CPU::INC_B()
{

	// Set the half carry flag if there is carry from bit 3, otherwise unset it
	((reg_BC.hi & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_BC.hi += 1;

	// Unset subtract flag if B is 0, set it otherwise
	reg_BC.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("INC B\n");
	return 4;
}

// DEC B
// Decrements the contents of B
int CPU::DEC_B()
{
	reg_BC.hi -= 1;

	// Set the zero flag if B is 0, unset it otherwise
	reg_BC.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set the half carry flag if there is borrow from bit 4, otherwise unset it
	((reg_BC.hi & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the subtract flag
	SET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("DEC B\n");
	return 4;
}

// LD B, u8
// Loads an 8 bit immediate value into B
int CPU::LD_B_u8()
{
	reg_BC.hi = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	debugPrint("LD B, u8\n");
	return 8;
}

// RLCA
// Rotate A left
int CPU::RLCA()
{
	UNSET_ZERO_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_AF.hi >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate A left by 1
	reg_AF.hi = (reg_AF.hi << 1) | (reg_AF.hi >> 7);

	reg_PC.dat += 1;
	debugPrint("RLCA\n");
	return 4;
}

// LD (u16), SP
// Loads the contents of SP into the memory address pointed to by the next 2 bytes
int CPU::LD_u16_SP()
{
	// Load the next 2 bytes into a 16 bit value
	// Left shift the second byte by 8 bits
	// OR the first byte
	// Due to endianness
	Word address = ((*mMap)[reg_PC.dat + 2] << 8) | (*mMap)[reg_PC.dat + 1];

	// Write the contents of SP into the memory address pointed to by the next 2 bytes
	mMap->writeMemory(address, reg_SP.lo);
	mMap->writeMemory(address + 1, reg_SP.hi);

	// Increment the program counter
	reg_PC.dat += 3;

	debugPrint("LD (u16), SP\n");
	return 20;
}

// ADD HL, BC
// Adds the contents of BC to HL
int CPU::ADD_HL_BC()
{
	// Set the half carry flag if there is carry from bit 11, otherwise unset it
	// TODO: profile (a) ? vs (a>>11) ?. byte is 0 or bit is 0 with bit shifts
	((reg_HL.dat & 0x0FFF) + (reg_BC.dat & 0x0FFF)) & 0x1000 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Word temp = reg_HL.dat;

	reg_HL.dat += reg_BC.dat;

	// Set carry flag if overflow from a word temp
	(reg_HL.dat < temp) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD HL, BC\n");
	return 8;
}

// LD A, (BC)
// Loads the contents of the memory address pointed to by BC into A
int CPU::LD_A_BC()
{
	reg_AF.hi = (*mMap)[reg_BC.dat];
	reg_PC.dat += 1;
	debugPrint("LD A, (BC)\n");
	return 8;
}

// DEC BC
// Decrement BC
int CPU::DEC_BC()
{
	reg_BC.dat -= 1;
	reg_PC.dat += 1;
	debugPrint("DEC BC\n");
	return 8;
}

// INC C
// Increment C
int CPU::INC_C()
{
	// Set the half carry flag if there is carry from bit 3, otherwise unset it
	((reg_BC.lo & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_BC.lo += 1;

	// Unset subtract flag if B is 0, set it otherwise
	reg_BC.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("INC C\n");
	return 4;
}

// DEC C
// Decrement C
int CPU::DEC_C()
{
	reg_BC.lo -= 1;

	// Set the zero flag if B is 0, unset it otherwise
	reg_BC.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set the half carry flag if there is borrow from bit 4, otherwise unset it
	((reg_BC.lo & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the subtract flag
	SET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("DEC C\n");
	return 4;
}

// LD C, u8
// Loads an 8 bit immediate value into C
int CPU::LD_C_u8()
{
	reg_BC.lo = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	debugPrint("LD C, u8\n");
	return 8;
}

// RRCA
// Rotate A right
int CPU::RRCA()
{
	// Unset zero flag
	UNSET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set carry flag if bit 0 is set, unset it otherwise
	(reg_AF.hi & 1) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Unset half carry flag
	UNSET_HALF_CARRY_FLAG;

	// Rotate A right by 1
	reg_AF.hi = (reg_AF.hi >> 1) | (reg_AF.hi << 7);

	reg_PC.dat += 1;
	debugPrint("RRCA\n");
	return 4;
}

// STOP
// Stops the CPU until an interrupt occurs
int CPU::STOP()
{
	isLowPower = true;
	reg_PC.dat += 2;
	debugPrint("STOP\n");
	return 0;
}

// LD DE, u16
// Loads a 16 bit immediate value into DE
int CPU::LD_DE_u16()
{
	reg_DE.dat = ((*mMap)[reg_PC.dat + 2] << 8) | (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 3;
	debugPrint("LD DE, u16\n");
	return 12;
}

// LD (DE), A
// Loads the contents of A into the memory address pointed to by DE
int CPU::LD_DE_A()
{
	mMap->writeMemory(reg_DE.dat, reg_AF.hi);
	reg_PC.dat += 1;
	debugPrint("LD (DE), A\n");
	return 8;
}

// INC DE
// Increment DE
int CPU::INC_DE()
{
	reg_DE.dat += 1;
	reg_PC.dat += 1;
	debugPrint("INC DE\n");
	return 8;
}

// INC D
// Increment D
int CPU::INC_D()
{
	// Set the half carry flag if there is carry from bit 3, otherwise unset it
	((reg_DE.hi & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_DE.hi += 1;

	// Unset subtract flag if B is 0, set it otherwise
	reg_DE.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("INC D\n");
	return 4;
}

// DEC D
// Decrement D
int CPU::DEC_D()
{
	reg_DE.hi -= 1;

	// Set the zero flag if B is 0, unset it otherwise
	reg_DE.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set the half carry flag if there is borrow from bit 4, otherwise unset it
	((reg_DE.hi & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the subtract flag
	SET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("DEC D\n");
	return 4;
}

// LD D, u8
// Loads an 8 bit immediate value into D
int CPU::LD_D_u8()
{
	reg_DE.hi = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	debugPrint("LD D, u8\n");
	return 8;
}

// RLA
// Rotate A left through carry flag
int CPU::RLA()
{
	// Unset zero flag
	UNSET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Unsset half carry flag
	UNSET_HALF_CARRY_FLAG;

	Byte tempCarry = GET_CARRY_FLAG;

	reg_AF.hi >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift A left by 1
	reg_AF.hi = (reg_AF.hi << 1) | (tempCarry);

	reg_PC.dat += 1;
	debugPrint("RLA\n");
	return 4;
}

// JR i8
// Add a signed 8 bit immediate value to the program counter
int CPU::JR_i8()
{
	reg_PC.dat += (SByte)(*mMap)[reg_PC.dat + 1] + 2;
	debugPrint("JR i8\n");
	return 12;
}

// ADD HL, DE
// Add DE to HL
int CPU::ADD_HL_DE()
{
	// Set the half carry flag if there is carry from bit 11, otherwise unset it
	// TODO: profile (a) ? vs (a>>11) ?. byte is 0 or bit is 0 with bit shifts
	((reg_HL.dat & 0x0FFF) + (reg_DE.dat & 0x0FFF)) & 0x1000 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Word temp = reg_HL.dat;

	reg_HL.dat += reg_DE.dat;

	// Set carry flag if overflow from a word temp
	(reg_HL.dat < temp) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD HL, DE\n");
	return 8;
}

// LD A, (DE)
// Loads the contents of the memory address pointed to by DE into A
int CPU::LD_A_DE()
{
	reg_AF.hi = (*mMap)[reg_DE.dat];
	reg_PC.dat += 1;
	debugPrint("LD A, (DE)\n");
	return 8;
}

// DEC DE
// Decrement DE
int CPU::DEC_DE()
{
	reg_DE.dat -= 1;
	reg_PC.dat += 1;
	debugPrint("DEC DE\n");
	return 8;
}

// INC E
// Increment E
int CPU::INC_E()
{
	// Set the half carry flag if there is carry from bit 3, otherwise unset it
	((reg_DE.lo & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_DE.lo += 1;

	// Unset subtract flag if B is 0, set it otherwise
	reg_DE.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("INC E\n");
	return 4;
}

// DEC E
// Decrement E
int CPU::DEC_E()
{
	reg_DE.lo -= 1;

	// Set the zero flag if B is 0, unset it otherwise
	reg_DE.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set the half carry flag if there is borrow from bit 4, otherwise unset it
	((reg_DE.lo & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the subtract flag
	SET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("DEC E\n");
	return 4;
}

// LD E, u8
// Loads an 8 bit immediate value into E
int CPU::LD_E_u8()
{
	reg_DE.lo = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	debugPrint("LD E, u8\n");
	return 8;
}

// RRA
// Rotate A right through carry flag
int CPU::RRA()
{
	// Unset zero flag
	UNSET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Unset Half Carry flag
	UNSET_HALF_CARRY_FLAG;

	bool tempCarry = GET_CARRY_FLAG;

	// Set Carry flag to 1 if bit 0 is 1
	// Example: 1000 0001 will become 0100 0000
	(reg_AF.hi & 0x01) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift A right by 1
	reg_AF.hi = (reg_AF.hi >> 1) | (tempCarry << 7);

	reg_PC.dat += 1;
	debugPrint("RRA\n");
	return 4;
}

// JR NZ, i8
// Add a signed 8 bit immediate value to the program counter if zero flag is 0
// 3 cycles if taken, 2 cycles if not taken
int CPU::JR_NZ_i8()
{
	debugPrint("JR NZ, i8\n");

	if (!(reg_AF.lo & FLAG_ZERO_z))
	{
		reg_PC.dat += (SByte)(*mMap)[reg_PC.dat + 1] + 2;
		return 12;
	}

	reg_PC.dat += 2;
	return 8;
}

// LD HL, u16
// Loads a 16 bit immediate value into HL
int CPU::LD_HL_u16()
{
	reg_HL.dat = ((*mMap)[reg_PC.dat + 2] << 8) | (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 3;
	debugPrint("LD HL, u16\n");
	return 12;
}

// LD (HL+), A
// Loads the contents of A into the memory address pointed to by HL and increments HL
int CPU::LD_HLp_A()
{
	mMap->writeMemory(reg_HL.dat, reg_AF.hi);
	reg_HL.dat += 1;
	reg_PC.dat += 1;
	debugPrint("LD (HL+), A\n");
	return 8;
}

// INC HL
// Increment HL
int CPU::INC_HL()
{
	reg_HL.dat += 1;
	reg_PC.dat += 1;
	debugPrint("INC HL\n");
	return 8;
}

// INC H
// Increment H
int CPU::INC_H()
{
	// Set the half carry flag if there is carry from bit 3, otherwise unset it
	((reg_HL.hi & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_HL.hi += 1;

	// Unset subtract flag if B is 0, set it otherwise
	reg_HL.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("INC H\n");
	return 4;
}

// DEC H
// Decrement H
int CPU::DEC_H()
{
	reg_HL.hi -= 1;

	// Set the zero flag if B is 0, unset it otherwise
	reg_HL.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set the half carry flag if there is borrow from bit 4, otherwise unset it
	((reg_HL.hi & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the subtract flag
	SET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("DEC H\n");
	return 4;
}

// LD H, u8
// Loads an 8 bit immediate value into H
int CPU::LD_H_u8()
{
	reg_HL.hi = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	debugPrint("LD H, u8\n");
	return 8;
}

// DAA
// Decimal adjust register A
int CPU::DAA()
{
	if (!GET_SUBTRACT_FLAG)
	{
		if (GET_CARRY_FLAG || reg_AF.hi > 0x99)
		{
			reg_AF.hi += 0x60;
			SET_CARRY_FLAG;
		}
		if (GET_HALF_CARRY_FLAG || (reg_AF.hi & 0x0F) > 0x09)
		{
			reg_AF.hi += 0x06;
		}
	}
	else if (GET_CARRY_FLAG && GET_HALF_CARRY_FLAG)
	{
		reg_AF.hi += 0x9A;
	}
	else if (GET_CARRY_FLAG)
	{
		reg_AF.hi += 0xA0;
	}
	else if (GET_HALF_CARRY_FLAG)
	{
		reg_AF.hi += 0xFA;
	}

	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;
	UNSET_HALF_CARRY_FLAG;

	debugPrint("DAA\n");

	reg_PC.dat += 1;

	return 4;
}

// JR Z, i8
// Add a signed 8 bit immediate value to the program counter if zero flag is 1
// 3 cycles if taken, 2 cycles if not taken
int CPU::JR_Z_r8()
{
	debugPrint("JR Z, i8\n");
	if (reg_AF.lo & FLAG_ZERO_z)
	{
		reg_PC.dat += (SByte)(*mMap)[reg_PC.dat + 1] + 2;
		return 12;
	}
	reg_PC.dat += 2;

	return 8;
}

// ADD HL, HL
// Add HL to HL
int CPU::ADD_HL_HL()
{
	// Set subtract flag to 0
	UNSET_SUBTRACT_FLAG;

	// Set half carry flag to 1 if there was a carry from bit 11
	// Example: 0000 1000 0000 0000 + 0000 1000 0000 0000 = 0001 0000 0000 0000
	// TODO: profile (a) ? vs (a>>11) ? byte is 0 or bit is 0 with bit shifts
	reg_HL.dat & 0x0800 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag to 1 if there was a carry from bit 15
	// Example: 1000 0000 0000 0000 + 1000 0000 0000 0000 = 0000 0000 0000 0000
	reg_HL.dat & 0x8000 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_HL.dat += reg_HL.dat;
	reg_PC.dat += 1;
	debugPrint("ADD HL, HL\n");
	return 8;
}

// LD A, (HL+)
// Loads the contents of the memory address pointed to by HL into A and increments HL
int CPU::LD_A_HLp()
{
	reg_AF.hi = (*mMap)[reg_HL.dat];
	reg_HL.dat += 1;
	reg_PC.dat += 1;
	debugPrint("LD A, (HL+)\n");
	return 8;
}

// DEC HL
// Decrement HL
int CPU::DEC_HL()
{
	reg_HL.dat -= 1;
	reg_PC.dat += 1;
	debugPrint("DEC HL\n");
	return 8;
}

// INC L
// Increment L
int CPU::INC_L()
{
	// Set the half carry flag if there is carry from bit 3, otherwise unset it
	((reg_HL.lo & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_HL.lo += 1;

	// Unset subtract flag if B is 0, set it otherwise
	reg_HL.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("INC L\n");
	return 4;
}

// DEC L
// Decrement L
int CPU::DEC_L()
{
	reg_HL.lo -= 1;

	// Set the zero flag if B is 0, unset it otherwise
	reg_HL.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set the half carry flag if there is borrow from bit 4, otherwise unset it
	((reg_HL.lo & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the subtract flag
	SET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("DEC L\n");
	return 4;
}

// LD L, u8
// Loads an 8 bit immediate value into L
int CPU::LD_L_u8()
{
	reg_HL.lo = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	debugPrint("LD L, u8\n");
	return 8;
}

// CPL
// Complement A
int CPU::CPL()
{
	reg_AF.hi = ~reg_AF.hi;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	// Set half carry flag
	SET_HALF_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("CPL\n");
	return 4;
}

// JR NC, i8
// Add a signed 8 bit immediate value to the program counter if carry flag is 0
// 3 cycles if condition is true, 2 otherwise
int CPU::JR_NC_i8()
{
	debugPrint("JR NC, i8\n");
	if (!(reg_AF.lo & FLAG_CARRY_c))
	{
		reg_PC.dat += (SByte)(*mMap)[reg_PC.dat + 1] + 2;
		return 12;
	}
	reg_PC.dat += 2;

	return 8;
}

// LD SP, u16
// Loads a 16 bit immediate value into SP
int CPU::LD_SP_u16()
{
	reg_SP.dat = ((*mMap)[reg_PC.dat + 2] << 8) | (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 3;
	debugPrint("LD SP, u16\n");
	return 12;
}

// LD (HL-), A
// Loads the contents of A into the memory address pointed to by HL and decrements HL
int CPU::LD_HLm_A()
{
	mMap->writeMemory(reg_HL.dat, reg_AF.hi);
	reg_HL.dat -= 1;
	reg_PC.dat += 1;
	debugPrint("LD (HL-), A\n");
	return 8;
}

// INC SP
// Increment SP
int CPU::INC_SP()
{
	reg_SP.dat += 1;
	reg_PC.dat += 1;
	debugPrint("INC SP\n");
	return 8;
}

// INC (HL)
// Increment the contents of the memory address pointed to by HL
int CPU::INC_HLp()
{
	Byte temp = (*mMap)[reg_HL.dat];
	temp += 1;

	// Unset zero flag if B is 0, set it otherwise
	temp ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set the half carry flag if there is carry from bit 3, otherwise unset it
	((*mMap)[reg_HL.dat] & 0x0F) == 0x0F ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	mMap->writeMemory(reg_HL.dat, temp);
	reg_PC.dat += 1;
	debugPrint("INC (HL)\n");
	return 12;
}

// DEC (HL)
// Decrement the contents of the memory address pointed to by HL
int CPU::DEC_HLp()
{
	Byte temp = (*mMap)[reg_HL.dat];

	temp -= 1;

	// Set the half carry flag if there is borrow from bit 4, otherwise unset it
	((temp & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the zero flag if B is 0, unset it otherwise
	temp ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set the subtract flag
	SET_SUBTRACT_FLAG;

	mMap->writeMemory(reg_HL.dat, temp);
	reg_PC.dat += 1;
	debugPrint("DEC (HL)\n");
	return 12;
}

// LD (HL), u8
// Loads an 8 bit immediate value into the memory address pointed to by HL
int CPU::LD_HLp_u8()
{
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_PC.dat + 1]);
	reg_PC.dat += 2;
	debugPrint("LD (HL), u8\n");
	return 12;
}

// SCF
// Set carry flag
int CPU::SCF()
{
	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Unset half carry flag
	UNSET_HALF_CARRY_FLAG;

	// Set carry flag
	SET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("SCF\n");
	return 4;
}

// JR C, i8
// Add a signed 8 bit immediate value to the program counter if carry flag is 1
// 3 cycles if condition is true, 2 otherwise
int CPU::JR_C_r8()
{
	debugPrint("JR C, i8\n");
	if (reg_AF.lo & FLAG_CARRY_c)
	{
		reg_PC.dat += (SByte)(*mMap)[reg_PC.dat + 1] + 2;
		return 12;
	}
	reg_PC.dat += 2;

	return 8;
}

// ADD HL, SP
// Add SP to HL
int CPU::ADD_HL_SP()
{
	// Set the half carry flag if there is carry from bit 11, otherwise unset it
	// TODO: profile (a) ? vs (a>>11) ?. byte is 0 or bit is 0 with bit shifts
	((reg_HL.dat & 0x0FFF) + (reg_SP.dat & 0x0FFF)) & 0x1000 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Word temp = reg_HL.dat;

	reg_HL.dat += reg_SP.dat;

	// Set carry flag if overflow from a word temp
	(reg_HL.dat < temp) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD HL, SP\n");
	return 8;
}

// LD A, (HL-)
// Loads the contents of the memory address pointed to by HL into A and decrements HL
int CPU::LD_A_HLm()
{
	reg_AF.hi = (*mMap)[reg_HL.dat];
	reg_HL.dat -= 1;
	reg_PC.dat += 1;
	debugPrint("LD A, (HL-)\n");
	return 8;
}

// DEC SP
// Decrement SP
int CPU::DEC_SP()
{
	reg_SP.dat -= 1;
	reg_PC.dat += 1;
	debugPrint("DEC SP\n");
	return 8;
}

// INC A
// Increment A
int CPU::INC_A()
{
	// Set the half carry flag if there is carry from bit 3, otherwise unset it
	(reg_AF.hi & 0x0F) == 0x0F ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_AF.hi += 1;

	// Unset subtract flag if B is 0, set it otherwise
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("INC A\n");
	return 4;
}

// DEC A
// Decrement A
int CPU::DEC_A()
{
	reg_AF.hi -= 1;

	// Set the half carry flag if there is borrow from bit 4, otherwise unset it
	((reg_AF.hi & 0x0F) == 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set the zero flag if B is 0, unset it otherwise
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set the subtract flag
	SET_SUBTRACT_FLAG;

	reg_PC.dat += 1;
	debugPrint("DEC A\n");
	return 4;
}

// LD A, u8
// Loads an 8 bit immediate value into A
int CPU::LD_A_u8()
{
	reg_AF.hi = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	debugPrint("LD A, u8\n");
	return 8;
}

// CCF
// Complement carry flag
int CPU::CCF()
{
	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Unset half carry flag
	UNSET_HALF_CARRY_FLAG;

	// Complement carry flag
	reg_AF.lo& FLAG_CARRY_c ? UNSET_CARRY_FLAG : SET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("CCF\n");
	return 4;
}

// LD B, B
// Loads B into B
int CPU::LD_B_B()
{
	reg_PC.dat += 1;
	debugPrint("LD B, B\n");
	return 4;
}

// LD B, C
// Loads C into B
int CPU::LD_B_C()
{
	reg_BC.hi = reg_BC.lo;
	reg_PC.dat += 1;
	debugPrint("LD B, C\n");
	return 4;
}

// LD B, D
// Loads D into B
int CPU::LD_B_D()
{
	reg_BC.hi = reg_DE.hi;
	reg_PC.dat += 1;
	debugPrint("LD B, D\n");
	return 4;
}

// LD B, E
// Loads E into B
int CPU::LD_B_E()
{
	reg_BC.hi = reg_DE.lo;
	reg_PC.dat += 1;
	debugPrint("LD B, E\n");
	return 4;
}

// LD B, H
// Loads H into B
int CPU::LD_B_H()
{
	reg_BC.hi = reg_HL.hi;
	reg_PC.dat += 1;
	debugPrint("LD B, H\n");
	return 4;
}

// LD B, L
// Loads L into B
int CPU::LD_B_L()
{
	reg_BC.hi = reg_HL.lo;
	reg_PC.dat += 1;
	debugPrint("LD B, L\n");
	return 4;
}

// LD B, (HL)
// Loads the value at address HL into B
int CPU::LD_B_HLp()
{
	reg_BC.hi = (*mMap)[reg_HL.dat];
	reg_PC.dat += 1;
	debugPrint("LD B, (HL)\n");
	return 8;
}

// LD B, A
// Loads A into B
int CPU::LD_B_A()
{
	reg_BC.hi = reg_AF.hi;
	reg_PC.dat += 1;
	debugPrint("LD B, A\n");
	return 4;
}

// LD C, B
// Loads B into C
int CPU::LD_C_B()
{
	reg_BC.lo = reg_BC.hi;
	reg_PC.dat += 1;
	debugPrint("LD C, B\n");
	return 4;
}

// LD C, C
// Loads C into C
int CPU::LD_C_C()
{
	reg_BC.lo = reg_BC.lo;
	reg_PC.dat += 1;
	debugPrint("LD C, C\n");
	return 4;
}

// LD C, D
// Loads D into C
int CPU::LD_C_D()
{
	reg_BC.lo = reg_DE.hi;
	reg_PC.dat += 1;
	debugPrint("LD C, D\n");
	return 4;
}

// LD C, E
// Loads E into C
int CPU::LD_C_E()
{
	reg_BC.lo = reg_DE.lo;
	reg_PC.dat += 1;
	debugPrint("LD C, E\n");
	return 4;
}

// LD C, H
// Loads H into C
int CPU::LD_C_H()
{
	reg_BC.lo = reg_HL.hi;
	reg_PC.dat += 1;
	debugPrint("LD C, H\n");
	return 4;
}

// LD C, L
// Loads L into C
int CPU::LD_C_L()
{
	reg_BC.lo = reg_HL.lo;
	reg_PC.dat += 1;
	debugPrint("LD C, L\n");
	return 4;
}

// LD C, (HL)
// Loads the value at address HL into C
int CPU::LD_C_HLp()
{
	reg_BC.lo = (*mMap)[reg_HL.dat];
	reg_PC.dat += 1;
	debugPrint("LD C, (HL)\n");
	return 8;
}

// LD C, A
// Loads A into C
int CPU::LD_C_A()
{
	reg_BC.lo = reg_AF.hi;
	reg_PC.dat += 1;
	debugPrint("LD C, A\n");
	return 4;
}

// LD D, B
// Loads B into D
int CPU::LD_D_B()
{
	reg_DE.hi = reg_BC.hi;
	reg_PC.dat += 1;
	debugPrint("LD D, B\n");
	return 4;
}

// LD D, C
// Loads C into D
int CPU::LD_D_C()
{
	reg_DE.hi = reg_BC.lo;
	reg_PC.dat += 1;
	debugPrint("LD D, C\n");
	return 4;
}

// LD D, D
// Loads D into D
int CPU::LD_D_D()
{
	reg_PC.dat += 1;
	debugPrint("LD D, D\n");
	return 4;
}

// LD D, E
// Loads E into D
int CPU::LD_D_E()
{
	reg_DE.hi = reg_DE.lo;
	reg_PC.dat += 1;
	debugPrint("LD D, E\n");
	return 4;
}

// LD D, H
// Loads H into D
int CPU::LD_D_H()
{
	reg_DE.hi = reg_HL.hi;
	reg_PC.dat += 1;
	debugPrint("LD D, H\n");
	return 4;
}

// LD D, L
// Loads L into D
int CPU::LD_D_L()
{
	reg_DE.hi = reg_HL.lo;
	reg_PC.dat += 1;
	debugPrint("LD D, L\n");
	return 4;
}

// LD D, (HL)
// Loads the value at address HL into D
int CPU::LD_D_HLp()
{
	reg_DE.hi = (*mMap)[reg_HL.dat];
	reg_PC.dat += 1;
	debugPrint("LD D, (HL)\n");
	return 8;
}

// LD D, A
// Loads A into D
int CPU::LD_D_A()
{
	reg_DE.hi = reg_AF.hi;
	reg_PC.dat += 1;
	debugPrint("LD D, A\n");
	return 4;
}

// LD E, B
// Loads B into E
int CPU::LD_E_B()
{
	reg_DE.lo = reg_BC.hi;
	reg_PC.dat += 1;
	debugPrint("LD E, B\n");
	return 4;
}

// LD E, C
// Loads C into E
int CPU::LD_E_C()
{
	reg_DE.lo = reg_BC.lo;
	reg_PC.dat += 1;
	debugPrint("LD E, C\n");
	return 4;
}

// LD E, D
// Loads D into E
int CPU::LD_E_D()
{
	reg_DE.lo = reg_DE.hi;
	reg_PC.dat += 1;
	debugPrint("LD E, D\n");
	return 4;
}

// LD E, E
// Loads E into E
int CPU::LD_E_E()
{
	reg_PC.dat += 1;
	debugPrint("LD E, E\n");
	return 4;
}

// LD E, H
// Loads H into E
int CPU::LD_E_H()
{
	reg_DE.lo = reg_HL.hi;
	reg_PC.dat += 1;
	debugPrint("LD E, H\n");
	return 4;
}

// LD E, L
// Loads L into E
int CPU::LD_E_L()
{
	reg_DE.lo = reg_HL.lo;
	reg_PC.dat += 1;
	debugPrint("LD E, L\n");
	return 4;
}

// LD E, (HL)
// Loads the value at address HL into E
int CPU::LD_E_HLp()
{
	reg_DE.lo = (*mMap)[reg_HL.dat];
	reg_PC.dat += 1;
	debugPrint("LD E, (HL)\n");
	return 8;
}

// LD E, A
// Loads A into E
int CPU::LD_E_A()
{
	reg_DE.lo = reg_AF.hi;
	reg_PC.dat += 1;
	debugPrint("LD E, A\n");
	return 4;
}

// LD H, B
// Loads B into H
int CPU::LD_H_B()
{
	reg_HL.hi = reg_BC.hi;
	reg_PC.dat += 1;
	debugPrint("LD H, B\n");
	return 4;
}

// LD H, C
// Loads C into H
int CPU::LD_H_C()
{
	reg_HL.hi = reg_BC.lo;
	reg_PC.dat += 1;
	debugPrint("LD H, C\n");
	return 4;
}

// LD H, D
// Loads D into H
int CPU::LD_H_D()
{
	reg_HL.hi = reg_DE.hi;
	reg_PC.dat += 1;
	debugPrint("LD H, D\n");
	return 4;
}

// LD H, E
// Loads E into H
int CPU::LD_H_E()
{
	reg_HL.hi = reg_DE.lo;
	reg_PC.dat += 1;
	debugPrint("LD H, E\n");
	return 4;
}

// LD H, H
// Loads H into H
int CPU::LD_H_H()
{
	reg_PC.dat += 1;
	debugPrint("LD H, H\n");
	return 4;
}

// LD H, L
// Loads L into H
int CPU::LD_H_L()
{
	reg_HL.hi = reg_HL.lo;
	reg_PC.dat += 1;
	debugPrint("LD H, L\n");
	return 4;
}

// LD H, (HL)
// Loads the value at address HL into H
int CPU::LD_H_HLp()
{
	reg_HL.hi = (*mMap)[reg_HL.dat];
	reg_PC.dat += 1;
	debugPrint("LD H, (HL)\n");
	return 8;
}

// LD H, A
// Loads A into H
int CPU::LD_H_A()
{
	reg_HL.hi = reg_AF.hi;
	reg_PC.dat += 1;
	debugPrint("LD H, A\n");
	return 4;
}

// LD L, B
// Loads B into L
int CPU::LD_L_B()
{
	reg_HL.lo = reg_BC.hi;
	reg_PC.dat += 1;
	debugPrint("LD L, B\n");
	return 4;
}

// LD L, C
// Loads C into L
int CPU::LD_L_C()
{
	reg_HL.lo = reg_BC.lo;
	reg_PC.dat += 1;
	debugPrint("LD L, C\n");
	return 4;
}

// LD L, D
// Loads D into L
int CPU::LD_L_D()
{
	reg_HL.lo = reg_DE.hi;
	reg_PC.dat += 1;
	debugPrint("LD L, D\n");
	return 4;
}

// LD L, E
// Loads E into L
int CPU::LD_L_E()
{
	reg_HL.lo = reg_DE.lo;
	reg_PC.dat += 1;
	debugPrint("LD L, E\n");
	return 4;
}

// LD L, H
// Loads H into L
int CPU::LD_L_H()
{
	reg_HL.lo = reg_HL.hi;
	reg_PC.dat += 1;
	debugPrint("LD L, H\n");
	return 4;
}

// LD L, L
// Loads L into L
int CPU::LD_L_L()
{
	reg_PC.dat += 1;
	debugPrint("LD L, L\n");
	return 4;
}

// LD L, (HL)
// Loads the value at address HL into L
int CPU::LD_L_HLp()
{
	reg_HL.lo = (*mMap)[reg_HL.dat];
	reg_PC.dat += 1;
	debugPrint("LD L, (HL)\n");
	return 8;
}

// LD L, A
// Loads A into L
int CPU::LD_L_A()
{
	reg_HL.lo = reg_AF.hi;
	reg_PC.dat += 1;
	debugPrint("LD L, A\n");
	return 4;
}

// LD (HL), B
// Loads B into the value at address HL
int CPU::LD_HLp_B()
{
	mMap->writeMemory(reg_HL.dat, reg_BC.hi);
	reg_PC.dat += 1;
	debugPrint("LD (HL), B\n");
	return 8;
}

// LD (HL), C
// Loads C into the value at address HL
int CPU::LD_HLp_C()
{
	mMap->writeMemory(reg_HL.dat, reg_BC.lo);
	reg_PC.dat += 1;
	debugPrint("LD (HL), C\n");
	return 8;
}

// LD (HL), D
// Loads D into the value at address HL
int CPU::LD_HLp_D()
{
	mMap->writeMemory(reg_HL.dat, reg_DE.hi);
	reg_PC.dat += 1;
	debugPrint("LD (HL), D\n");
	return 8;
}

// LD (HL), E
// Loads E into the value at address HL
int CPU::LD_HLp_E()
{
	mMap->writeMemory(reg_HL.dat, reg_DE.lo);
	reg_PC.dat += 1;
	debugPrint("LD (HL), E\n");
	return 8;
}

// LD (HL), H
// Loads H into the value at address HL
int CPU::LD_HLp_H()
{
	mMap->writeMemory(reg_HL.dat, reg_HL.hi);
	reg_PC.dat += 1;
	debugPrint("LD (HL), H\n");
	return 8;
}

// LD (HL), L
// Loads L into the value at address HL
int CPU::LD_HLp_L()
{
	mMap->writeMemory(reg_HL.dat, reg_HL.lo);
	reg_PC.dat += 1;
	debugPrint("LD (HL), L\n");
	return 8;
}

// HALT
// Halts the CPU until an interrupt occurs (Low Power Mode)
// If interrupts are disabled, don't go in Low power and skip next byte
int CPU::HALT()
{

	// The HALT BUG
	// iF IME = 0 and IE & IF != 0
	// the halt bug occurs where CPU reads the next byte twice
	// or more aptly, PC fails to increment (above statement is refuted in case of halt after halt)
	// Low Power Mode is NOT entered in this case

	// IMPORTANT - POTENTIAL BUG SOURCE
	// My implementation of the halt bug here is to
	// decrement PC, executeInstruction manually and return
	// 4 + whatever the next opcode returns
	// This skips on updateTimers, performInterrupts and updateGraphics
	// for one iteration, and might create problems in future
	// The other alternative is to have an if statement in executeNextInstruction
	// which is highly inefficient, so will go with this for now

	// Another quirk of this bug is if
	// HALT is caled just after EI
	// The interrupt is handled, but returned back to HALT
	// so HALT gets called twice

	if (!IMEReg & (mMap->getRegIE() & mMap->getRegIF()))
	{
		// Check if EI executed just before HALT
		// Pass through without a PC increment if true
		if (IMEFlag == 1)
			return 4;
		return 4 + executeInstruction((*mMap)[reg_PC.dat + 1]);
	}

	// If interrupts are enabled, go in HALT mode
	// Which is low power mode, but I made another bool
	// to differentiate from STOP behaviour
	// If interrupts are disabled, skip the next byte
	isHalted = true;

	return 4;
}

// LD (HL), A
// Loads A into the value at address HL
int CPU::LD_HLA()
{
	mMap->writeMemory(reg_HL.dat, reg_AF.hi);
	reg_PC.dat += 1;
	debugPrint("LD (HL), A\n");
	return 8;
}

// LD A, B
// Loads B into A
int CPU::LD_A_B()
{
	reg_AF.hi = reg_BC.hi;
	reg_PC.dat += 1;
	debugPrint("LD A, B\n");
	return 4;
}

// LD A, C
// Loads C into A
int CPU::LD_A_C()
{
	reg_AF.hi = reg_BC.lo;
	reg_PC.dat += 1;
	debugPrint("LD A, C\n");
	return 4;
}

// LD A, D
// Loads D into A
int CPU::LD_A_D()
{
	reg_AF.hi = reg_DE.hi;
	reg_PC.dat += 1;
	debugPrint("LD A, D\n");
	return 4;
}

// LD A, E
// Loads E into A
int CPU::LD_A_E()
{
	reg_AF.hi = reg_DE.lo;
	reg_PC.dat += 1;
	debugPrint("LD A, E\n");
	return 4;
}

// LD A, H
// Loads H into A
int CPU::LD_A_H()
{
	reg_AF.hi = reg_HL.hi;
	reg_PC.dat += 1;
	debugPrint("LD A, H\n");
	return 4;
}

// LD A, L
// Loads L into A
int CPU::LD_A_L()
{
	reg_AF.hi = reg_HL.lo;
	reg_PC.dat += 1;
	debugPrint("LD A, L\n");
	return 4;
}

// LD A, (HL)
// Loads the value at address HL into A
int CPU::LD_A_HL()
{
	reg_AF.hi = (*mMap)[reg_HL.dat];
	reg_PC.dat += 1;
	debugPrint("LD A, (HL)\n");
	return 8;
}

// LD A, A
// Loads A into A
int CPU::LD_A_A()
{
	reg_PC.dat += 1;
	debugPrint("LD A, A\n");
	return 4;
}

// ADD A, B
// Adds B to A
int CPU::ADD_A_B()
{
	UNSET_SUBTRACT_FLAG;

	// Set zero flag if result is zero
	(reg_AF.hi + reg_BC.hi) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_BC.hi & 0x0F)) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if carry from bit 7
	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Byte temp = reg_AF.hi;

	reg_AF.hi += reg_BC.hi;

	// Set carry flag if overflow from a byte temp
	temp > reg_AF.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD A, B\n");
	return 4;
}

// ADD A, C
// Adds C to A
int CPU::ADD_A_C()
{
	UNSET_SUBTRACT_FLAG;

	// Set zero flag if result is zero
	(reg_AF.hi + reg_BC.lo) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_BC.lo & 0x0F)) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if carry from bit 7
	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Byte temp = reg_AF.hi;

	reg_AF.hi += reg_BC.lo;

	// Set carry flag if overflow from a byte temp
	temp > reg_AF.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD A, C\n");
	return 4;
}

// ADD A, D
// Adds D to A
int CPU::ADD_A_D()
{
	UNSET_SUBTRACT_FLAG;

	// Set zero flag if result is zero
	(reg_AF.hi + reg_DE.hi) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_DE.hi & 0x0F)) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if carry from bit 7
	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Byte temp = reg_AF.hi;

	reg_AF.hi += reg_DE.hi;

	// Set carry flag if overflow from a byte temp
	temp > reg_AF.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD A, D\n");
	return 4;
}

// ADD A, E
// Adds E to A
int CPU::ADD_A_E()
{
	UNSET_SUBTRACT_FLAG;

	// Set zero flag if result is zero
	(reg_AF.hi + reg_DE.lo) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_DE.lo & 0x0F)) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if carry from bit 7
	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Byte temp = reg_AF.hi;

	reg_AF.hi += reg_DE.lo;

	// Set carry flag if overflow from a byte temp
	temp > reg_AF.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD A, E\n");
	return 4;
}

// ADD A, H
// Adds H to A
int CPU::ADD_A_H()
{
	UNSET_SUBTRACT_FLAG;

	// Set zero flag if result is zero
	(reg_AF.hi + reg_HL.hi) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_HL.hi & 0x0F)) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if carry from bit 7
	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Byte temp = reg_AF.hi;

	reg_AF.hi += reg_HL.hi;

	// Set carry flag if overflow from a byte temp
	temp > reg_AF.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD A, H\n");
	return 4;
}

// ADD A, L
// Adds L to A
int CPU::ADD_A_L()
{
	UNSET_SUBTRACT_FLAG;

	// Set zero flag if result is zero
	(reg_AF.hi + reg_HL.lo) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_HL.lo & 0x0F)) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if carry from bit 7
	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Byte temp = reg_AF.hi;

	reg_AF.hi += reg_HL.lo;

	// Set carry flag if overflow from a byte temp
	temp > reg_AF.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD A, L\n");
	return 4;
}

// ADD A, (HL)
// Adds the value at address HL to A
int CPU::ADD_A_HLp()
{
	UNSET_SUBTRACT_FLAG;

	// Set zero flag if result is zero
	(reg_AF.hi + (*mMap)[reg_HL.dat]) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + ((*mMap)[reg_HL.dat] & 0x0F)) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if carry from bit 7
	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Byte temp = reg_AF.hi;

	reg_AF.hi += (*mMap)[reg_HL.dat];

	// Set carry flag if overflow from a byte temp
	temp > reg_AF.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD A, (HL)\n");
	return 8;
}

// ADD A, A
// Adds A to A
int CPU::ADD_A_A()
{
	UNSET_SUBTRACT_FLAG;

	// Set zero flag if result is zero
	(reg_AF.hi + reg_AF.hi) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set carry flag if carry from bit 7
	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Byte temp = reg_AF.hi;

	((reg_AF.hi & 0x0F) + (reg_AF.hi & 0x0F)) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_AF.hi += reg_AF.hi;

	// Set carry flag if overflow from a byte temp
	temp > reg_AF.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADD A, A\n");
	return 4;
}

// ADC A, B
// Adds B to A with carry
int CPU::ADC_A_B()
{
	// Set zero flag if result is zero
	(reg_AF.hi + reg_BC.hi + GET_CARRY_FLAG) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_BC.hi & 0x0F) + GET_CARRY_FLAG) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set carry flag if carry from bit 7
	Word temp = reg_AF.hi + GET_CARRY_FLAG + reg_BC.hi;

	reg_AF.hi += reg_BC.hi + GET_CARRY_FLAG;

	// Set carry flag if overflow from a byte temp
	temp > 0xFF ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADC A, B\n");
	return 4;
}

// ADC A, C
// Adds C to A with carry
int CPU::ADC_A_C()
{
	// Set zero flag if result is zero
	(reg_AF.hi + reg_BC.lo + GET_CARRY_FLAG) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_BC.lo & 0x0F) + GET_CARRY_FLAG) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set carry flag if carry from bit 7
	Word temp = reg_AF.hi + GET_CARRY_FLAG + reg_BC.lo;

	reg_AF.hi += reg_BC.lo + GET_CARRY_FLAG;

	// Set carry flag if overflow from a byte temp
	temp > 0xFF ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADC A, D\n");
	return 4;
}

// ADC A, D
// Adds D to A with carry
int CPU::ADC_A_D()
{
	// Set zero flag if result is zero
	(reg_AF.hi + reg_DE.hi + GET_CARRY_FLAG) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_DE.hi & 0x0F) + GET_CARRY_FLAG) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set carry flag if carry from bit 7
	Word temp = reg_AF.hi + GET_CARRY_FLAG + reg_DE.hi;

	reg_AF.hi += reg_DE.hi + GET_CARRY_FLAG;

	// Set carry flag if overflow from a byte temp
	temp > 0xFF ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADC A, D\n");
	return 4;
}

// ADC A, E
// Adds E to A with carry
int CPU::ADC_A_E()
{
	// Set zero flag if result is zero
	(reg_AF.hi + reg_DE.lo + GET_CARRY_FLAG) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_DE.lo & 0x0F) + GET_CARRY_FLAG) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set carry flag if carry from bit 7
	Word temp = reg_AF.hi + GET_CARRY_FLAG + reg_DE.lo;

	reg_AF.hi += reg_DE.lo + GET_CARRY_FLAG;

	// Set carry flag if overflow from a byte temp
	temp > 0xFF ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADC A, E\n");
	return 4;
}

// ADC A, H
// Adds H to A with carry
int CPU::ADC_A_H()
{
	// Set zero flag if result is zero
	(reg_AF.hi + reg_HL.hi + GET_CARRY_FLAG) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_HL.hi & 0x0F) + GET_CARRY_FLAG) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set carry flag if carry from bit 7
	// Set the carry flag if there is carry from bit 15, otherwise unset it
	Word temp = reg_AF.hi + GET_CARRY_FLAG + reg_HL.hi;

	reg_AF.hi += reg_HL.hi + GET_CARRY_FLAG;

	// Set carry flag if overflow from a byte temp
	temp > 0xFF ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADC A, H\n");
	return 4;
}

// ADC A, L
// Adds L to A with carry
int CPU::ADC_A_L()
{
	// Set zero flag if result is zero
	(reg_AF.hi + reg_HL.lo + GET_CARRY_FLAG) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_HL.lo & 0x0F) + GET_CARRY_FLAG) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set carry flag if carry from bit 7
	Word temp = reg_AF.hi + GET_CARRY_FLAG + reg_HL.lo;

	reg_AF.hi += reg_HL.lo + GET_CARRY_FLAG;

	// Set carry flag if overflow from a byte temp
	temp > 0xFF ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADC A, L\n");
	return 4;
}

// ADC A, (HL)
int CPU::ADC_A_HLp()
{
	// Set zero flag if result is zero
	(reg_AF.hi + (*mMap)[reg_HL.dat] + GET_CARRY_FLAG) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + ((*mMap)[reg_HL.dat] & 0x0F) + GET_CARRY_FLAG) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set carry flag if carry from bit 7
	Word temp = reg_AF.hi + GET_CARRY_FLAG + (*mMap)[reg_HL.dat];

	reg_AF.hi += (*mMap)[reg_HL.dat] + GET_CARRY_FLAG;

	// Set carry flag if overflow from a byte temp
	temp > 0xFF ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADC A, (HL)\n");
	return 8;
}

// ADC A, A
// Adds A to A with carry
int CPU::ADC_A_A()
{
	// Set zero flag if result is zero
	(reg_AF.hi + reg_AF.hi + GET_CARRY_FLAG) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if carry from bit 3
	((reg_AF.hi & 0x0F) + (reg_AF.hi & 0x0F) + GET_CARRY_FLAG) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set carry flag if carry from bit 7
	Word temp = reg_AF.hi + GET_CARRY_FLAG + reg_AF.hi;

	reg_AF.hi += reg_AF.hi + GET_CARRY_FLAG;

	// Set carry flag if overflow from a byte temp
	temp > 0xFF ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("ADC A, A\n");
	return 4;
}

// SUB A, B
// Subtracts B from A
int CPU::SUB_A_B()
{
	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_BC.hi & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	// Set carry flag if borrow from bit 7
	Byte temp = reg_AF.hi;

	reg_AF.hi -= reg_BC.hi;

	// Set carry flag if overflow from a byte temp
	temp < reg_AF.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SUB A, B\n");
	return 4;
}

// SUB A, C
// Subtracts C from A
int CPU::SUB_A_C()
{
	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_BC.lo & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. C > A
	(reg_AF.hi < reg_BC.lo) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= reg_BC.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SUB A, C\n");
	return 4;
}

// SUB A, D
// Subtracts D from A
int CPU::SUB_A_D()
{
	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_DE.hi & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. D > A
	(reg_AF.hi < reg_DE.hi) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= reg_DE.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SUB A, D\n");
	return 4;
}

// SUB A, E
// Subtracts E from A
int CPU::SUB_A_E()
{
	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_DE.lo & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. E > A
	(reg_AF.hi < reg_DE.lo) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= reg_DE.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SUB A, E\n");
	return 4;
}

// SUB A, H
// Subtracts H from A
int CPU::SUB_A_H()
{
	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_HL.hi & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. H > A
	(reg_AF.hi < reg_HL.hi) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= reg_HL.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SUB A, H\n");
	return 4;
}

// SUB A, L
// Subtracts L from A
int CPU::SUB_A_L()
{
	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_HL.lo & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. L > A
	(reg_AF.hi < reg_HL.lo) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= reg_HL.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SUB A, L\n");
	return 4;
}

// SUB A, (HL)
// Subtracts value at address HL from A
int CPU::SUB_A_HLp()
{
	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < ((*mMap)[reg_HL.dat] & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. (HL) > A
	(reg_AF.hi < (*mMap)[reg_HL.dat]) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= (*mMap)[reg_HL.dat];

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SUB A, (HL)\n");
	return 8;
}

// SUB A, A
int CPU::SUB_A_A()
{
	// Set subtract flag
	SET_SUBTRACT_FLAG;

	// Set zero flag
	SET_ZERO_FLAG;

	// Unset half carry flag
	UNSET_HALF_CARRY_FLAG;

	// Unset carry flag
	UNSET_CARRY_FLAG;

	reg_AF.hi = 0;

	reg_PC.dat += 1;
	debugPrint("SUB A, A\n");
	return 4;
}

// SBC A, B
// Subtracts B + carry flag from A
int CPU::SBC_A_B()
{
	bool tempCarry = GET_CARRY_FLAG;

	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_BC.hi & 0x0F) + GET_CARRY_FLAG ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. B + C > A
	(reg_AF.hi < (reg_BC.hi + GET_CARRY_FLAG)) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= (reg_BC.hi + tempCarry);

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SBC A, B\n");
	return 4;
}

// SBC A, C
// Subtracts C + carry flag from A
int CPU::SBC_A_C()
{
	bool tempCarry = GET_CARRY_FLAG;

	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_BC.lo & 0x0F) + GET_CARRY_FLAG ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. C + C > A
	(reg_AF.hi < (reg_BC.lo + GET_CARRY_FLAG)) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= (reg_BC.lo + tempCarry);

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SBC A, C\n");
	return 4;
}

// SBC A, D
// Subtracts D + carry flag from A
int CPU::SBC_A_D()
{
	bool tempCarry = GET_CARRY_FLAG;

	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_DE.hi & 0x0F) + GET_CARRY_FLAG ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. D + C > A
	(reg_AF.hi < (reg_DE.hi + GET_CARRY_FLAG)) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= (reg_DE.hi + tempCarry);

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SBC A, D\n");
	return 4;
}

// SBC A, E
// Subtracts E + carry flag from A
int CPU::SBC_A_E()
{
	bool tempCarry = GET_CARRY_FLAG;

	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_DE.lo & 0x0F) + GET_CARRY_FLAG ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. E + C > A
	(reg_AF.hi < (reg_DE.lo + GET_CARRY_FLAG)) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= (reg_DE.lo + tempCarry);

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SBC A, E\n");
	return 4;
}

// SBC A, H
// Subtracts H + carry flag from A
int CPU::SBC_A_H()
{
	bool tempCarry = GET_CARRY_FLAG;

	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_HL.hi & 0x0F) + GET_CARRY_FLAG ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. H + C > A
	(reg_AF.hi < (reg_HL.hi + GET_CARRY_FLAG)) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= (reg_HL.hi + tempCarry);

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SBC A, H\n");
	return 4;
}

// SBC A, L
// Subtracts L + carry flag from A
int CPU::SBC_A_L()
{
	bool tempCarry = GET_CARRY_FLAG;

	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_HL.lo & 0x0F) + GET_CARRY_FLAG ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. L + C > A
	(reg_AF.hi < (reg_HL.lo + GET_CARRY_FLAG)) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= (reg_HL.lo + tempCarry);

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SBC A, L\n");
	return 4;
}

// SBC A, (HL)
// Subtracts value at address HL + carry flag from A
int CPU::SBC_A_HLp()
{
	bool tempCarry = GET_CARRY_FLAG;

	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < ((*mMap)[reg_HL.dat] & 0x0F) + GET_CARRY_FLAG ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. (HL) + C > A
	(reg_AF.hi < ((*mMap)[reg_HL.dat] + GET_CARRY_FLAG)) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= ((*mMap)[reg_HL.dat] + tempCarry);

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SBC A, (HL)\n");
	return 8;
}

// SBC A, A
// Subtracts A + carry flag from A
int CPU::SBC_A_A()
{
	bool tempCarry = GET_CARRY_FLAG;

	// Set half carry flag if borrow from bit 4
	(reg_AF.hi & 0x0F) < (reg_AF.hi & 0x0F) + GET_CARRY_FLAG ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if borrow from bit 7 i.e. A + C > A
	(reg_AF.hi < (reg_AF.hi + GET_CARRY_FLAG)) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set subtract flag
	SET_SUBTRACT_FLAG;

	reg_AF.hi -= (reg_AF.hi + tempCarry);

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SBC A, A\n");
	return 4;
}

// AND A, B
// Performs bitwise AND on A and B
int CPU::AND_A_B()
{
	// Set half carry flag
	SET_HALF_CARRY_FLAG;

	// Unset subtract and carry flags
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi &= reg_BC.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("AND A, B\n");
	return 4;
}

// AND A, C
// Performs bitwise AND on A and C
int CPU::AND_A_C()
{
	// Set half carry flag
	SET_HALF_CARRY_FLAG;

	// Unset subtract and carry flags
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi &= reg_BC.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("AND A, C\n");
	return 4;
}

// AND A, D
// Performs bitwise AND on A and D
int CPU::AND_A_D()
{
	// Set half carry flag
	SET_HALF_CARRY_FLAG;

	// Unset subtract and carry flags
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi &= reg_DE.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("AND A, D\n");
	return 4;
}

// AND A, E
// Performs bitwise AND on A and E
int CPU::AND_A_E()
{
	// Set half carry flag
	SET_HALF_CARRY_FLAG;

	// Unset subtract and carry flags
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi &= reg_DE.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("AND A, E\n");
	return 4;
}

// AND A, H
// Performs bitwise AND on A and H
int CPU::AND_A_H()
{
	// Set half carry flag
	SET_HALF_CARRY_FLAG;

	// Unset subtract and carry flags
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi &= reg_HL.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("AND A, H\n");
	return 4;
}

// AND A, L
// Performs bitwise AND on A and L
int CPU::AND_A_L()
{
	// Set half carry flag
	SET_HALF_CARRY_FLAG;

	// Unset subtract and carry flags
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi &= reg_HL.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("AND A, L\n");
	return 4;
}

// AND A, (HL)
// Performs bitwise AND on A and value at address HL
int CPU::AND_A_HLp()
{
	// Set half carry flag
	SET_HALF_CARRY_FLAG;

	// Unset subtract and carry flags
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi &= (*mMap)[reg_HL.dat];

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("AND A, (HL)\n");
	return 8;
}

// AND A, A
// Performs bitwise AND on A and A
int CPU::AND_A_A()
{
	// Set half carry flag
	SET_HALF_CARRY_FLAG;

	// Unset subtract and carry flags
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi &= reg_AF.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("AND A, A\n");
	return 4;
}

// XOR A, B
// Performs bitwise XOR on A and B
int CPU::XOR_A_B()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi ^= reg_BC.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("XOR A, B\n");
	return 4;
}

// XOR A, C
// Performs bitwise XOR on A and C
int CPU::XOR_A_C()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi ^= reg_BC.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("XOR A, C\n");
	return 4;
}

// XOR A, D
// Performs bitwise XOR on A and D
int CPU::XOR_A_D()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi ^= reg_DE.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("XOR A, D\n");
	return 4;
}

// XOR A, E
// Performs bitwise XOR on A and E
int CPU::XOR_A_E()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi ^= reg_DE.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("XOR A, E\n");
	return 4;
}

// XOR A, H
// Performs bitwise XOR on A and H
int CPU::XOR_A_H()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi ^= reg_HL.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("XOR A, H\n");
	return 4;
}

// XOR A, L
// Performs bitwise XOR on A and L
int CPU::XOR_A_L()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi ^= reg_HL.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("XOR A, L\n");
	return 4;
}

// XOR A, (HL)
// Performs bitwise XOR on A and value at address HL
int CPU::XOR_A_HLp()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi ^= (*mMap)[reg_HL.dat];

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("XOR A, (HL)\n");
	return 8;
}

// XOR A, A
// Performs bitwise XOR on A and A
int CPU::XOR_A_A()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi = 0;

	// Set zero flag if result is zero
	SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("XOR A, A\n");
	return 4;
}

// OR A, B
// Performs bitwise OR on A and B
int CPU::OR_A_B()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi |= reg_BC.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("OR A, B\n");
	return 4;
}

// OR A, C
// Performs bitwise OR on A and C
int CPU::OR_A_C()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi |= reg_BC.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("OR A, C\n");
	return 4;
}

// OR A, D
// Performs bitwise OR on A and D
int CPU::OR_A_D()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi |= reg_DE.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("OR A, D\n");
	return 4;
}

// OR A, E
// Performs bitwise OR on A and E
int CPU::OR_A_E()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi |= reg_DE.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("OR A, E\n");
	return 4;
}

// OR A, H
// Performs bitwise OR on A and H
int CPU::OR_A_H()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi |= reg_HL.hi;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("OR A, H\n");
	return 4;
}

// OR A, L
// Performs bitwise OR on A and L
int CPU::OR_A_L()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi |= reg_HL.lo;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("OR A, L\n");
	return 4;
}

// OR A, (HL)
// Performs bitwise OR on A and value at address HL
int CPU::OR_A_HLp()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	reg_AF.hi |= (*mMap)[reg_HL.dat];

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("OR A, (HL)\n");
	return 8;
}

// OR A, A
// Performs bitwise OR on A and A
int CPU::OR_A_A()
{
	// Unset half carry, subtract and carry flags
	UNSET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	// Set zero flag if result is zero
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("OR A, A\n");
	return 4;
}

// CP A, B
// Subtracts B from A and set flags accordingly, but don't store the result.
int CPU::CP_A_B()
{
	// Unset half carry and subtract flags
	SET_SUBTRACT_FLAG;

	// Set carry flag if A < B
	reg_AF.hi < reg_BC.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A == B
	reg_AF.hi == reg_BC.hi ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;

	// Set half carry flag if lower nibble of A < lower nibble of B
	(reg_AF.hi & 0x0F) < (reg_BC.hi & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("CP A, B\n");
	return 4;
}

// CP A, C
// Subtracts C from A and set flags accordingly, but don't store the result.
int CPU::CP_A_C()
{
	// Unset half carry and subtract flags
	SET_SUBTRACT_FLAG;

	// Set carry flag if A < C
	reg_AF.hi < reg_BC.lo ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A == C
	reg_AF.hi == reg_BC.lo ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;

	// Set half carry flag if lower nibble of A < lower nibble of C
	(reg_AF.hi & 0x0F) < (reg_BC.lo & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("CP A, C\n");
	return 4;
}

// CP A, D
// Subtracts D from A and set flags accordingly, but don't store the result.
int CPU::CP_A_D()
{
	// Unset half carry and subtract flags
	SET_SUBTRACT_FLAG;

	// Set carry flag if A < D
	reg_AF.hi < reg_DE.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A == D
	reg_AF.hi == reg_DE.hi ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;

	// Set half carry flag if lower nibble of A < lower nibble of D
	(reg_AF.hi & 0x0F) < (reg_DE.hi & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("CP A, D\n");
	return 4;
}

// CP A, E
// Subtracts E from A and set flags accordingly, but don't store the result.
int CPU::CP_A_E()
{
	// Unset half carry and subtract flags
	SET_SUBTRACT_FLAG;

	// Set carry flag if A < E
	reg_AF.hi < reg_DE.lo ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A == E
	reg_AF.hi == reg_DE.lo ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;

	// Set half carry flag if lower nibble of A < lower nibble of E
	(reg_AF.hi & 0x0F) < (reg_DE.lo & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("CP A, E\n");
	return 4;
}

// CP A, H
// Subtracts H from A and set flags accordingly, but don't store the result.
int CPU::CP_A_H()
{
	// Unset half carry and subtract flags
	SET_SUBTRACT_FLAG;

	// Set carry flag if A < H
	reg_AF.hi < reg_HL.hi ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A == H
	reg_AF.hi == reg_HL.hi ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;

	// Set half carry flag if lower nibble of A < lower nibble of H
	(reg_AF.hi & 0x0F) < (reg_HL.hi & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("CP A, H\n");
	return 4;
}

// CP A, L
// Subtracts L from A and set flags accordingly, but don't store the result.
int CPU::CP_A_L()
{
	// Unset half carry and subtract flags
	SET_SUBTRACT_FLAG;

	// Set carry flag if A < L
	reg_AF.hi < reg_HL.lo ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A == L
	reg_AF.hi == reg_HL.lo ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;

	// Set half carry flag if lower nibble of A < lower nibble of L
	(reg_AF.hi & 0x0F) < (reg_HL.lo & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("CP A, L\n");
	return 4;
}

// CP A, (HL)
// Subtracts value at address HL from A and set flags accordingly, but don't store the result.
int CPU::CP_A_HLp()
{
	// Unset half carry and subtract flags
	SET_SUBTRACT_FLAG;

	// Set carry flag if A < value at address HL
	reg_AF.hi < (*mMap)[reg_HL.dat] ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A == value at address HL
	reg_AF.hi == (*mMap)[reg_HL.dat] ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;

	// Set half carry flag if lower nibble of A < lower nibble of value at address HL
	(reg_AF.hi & 0x0F) < ((*mMap)[reg_HL.dat] & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("CP A, (HL)\n");
	return 8;
}

// CP A, A
// Subtracts A from A and set flags accordingly, but don't store the result.
int CPU::CP_A_A()
{
	// Unset half carry and subtract flags
	SET_SUBTRACT_FLAG;

	// Unset carry flag
	UNSET_CARRY_FLAG;

	// Set zero flag
	SET_ZERO_FLAG;

	// Set half carry flag
	UNSET_HALF_CARRY_FLAG;

	reg_PC.dat += 1;
	debugPrint("CP A, A\n");
	return 4;
}

// RET NZ
// Return if zero flag is not set.
int CPU::RET_NZ()
{
	if (!GET_ZERO_FLAG)
	{
		reg_PC.dat = (*mMap)[reg_SP.dat] | ((*mMap)[reg_SP.dat + 1] << 8);
		reg_SP.dat += 2;
		debugPrint("RET NZ\n");
		return 20;
	}
	else
	{
		reg_PC.dat += 1;
		debugPrint("RET NZ\n");
		return 8;
	}
}

// POP BC
// Pop two bytes off the stack and store them in BC.
int CPU::POP_BC()
{
	reg_BC.dat = (*mMap)[reg_SP.dat] | ((*mMap)[reg_SP.dat + 1] << 8);
	reg_SP.dat += 2;
	reg_PC.dat += 1;
	debugPrint("POP BC\n");
	return 12;
}

// JP NZ, u16
// Jump to address u16 if zero flag is not set.
int CPU::JP_NZ_u16()
{
	if (!GET_ZERO_FLAG)
	{
		reg_PC.dat = ((*mMap)[reg_PC.dat + 2] << 8) | ((*mMap)[reg_PC.dat + 1]);
		debugPrint("JP NZ, %04X\n", reg_PC.dat);
		return 16;
	}
	else
	{
		reg_PC.dat += 3;
		debugPrint("JP NZ, %04X\n", reg_PC.dat);
		return 12;
	}
}

// JP u16
// Jump to address u16.
int CPU::JP_u16()
{
	reg_PC.dat = ((*mMap)[reg_PC.dat + 2] << 8) | (*mMap)[reg_PC.dat + 1];
	debugPrint("JP %04X\n", reg_PC.dat);
	return 16;
}

// CALL NZ, u16
// Call subroutine at address u16 if zero flag is not set.
int CPU::CALL_NZ_u16()
{
	if (!GET_ZERO_FLAG)
	{
		mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 3) >> 8);
		mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 3) & 0xFF);
		reg_PC.dat = (*mMap)[reg_PC.dat + 1] | ((*mMap)[reg_PC.dat + 2] << 8);
		debugPrint("CALL NZ, %04X\n", reg_PC.dat);
		return 24;
	}
	else
	{
		reg_PC.dat += 3;
		debugPrint("CALL NZ, %04X\n", reg_PC.dat);
		return 12;
	}
}

// PUSH BC
// Push BC onto the stack.
int CPU::PUSH_BC()
{
	mMap->writeMemory(--reg_SP.dat, reg_BC.hi);
	mMap->writeMemory(--reg_SP.dat, reg_BC.lo);
	reg_PC.dat += 1;
	debugPrint("PUSH BC\n");
	return 16;
}

// ADD A, u8
// Add u8 to A and set flags accordingly.
int CPU::ADD_A_u8()
{
	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	// Set carry flag if A + u8 > 0xFF
	reg_AF.hi + (*mMap)[reg_PC.dat + 1] > 0xFF ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A + u8 == 0
	(reg_AF.hi + (*mMap)[reg_PC.dat + 1]) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if lower nibble of A + lower nibble of u8 > 0xF
	(reg_AF.hi & 0x0F) + ((*mMap)[reg_PC.dat + 1] & 0x0F) > 0xF ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_AF.hi += (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	debugPrint("ADD A, %02X\n", (*mMap)[reg_PC.dat + 1]);
	return 8;
}

// RST 00H
// Call subroutine at address 0x0000.
int CPU::RST_00H()
{
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) >> 8);
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) & 0xFF);
	reg_PC.dat = 0x0000;
	debugPrint("RST 00H\n");
	return 16;
}

// RET Z
// Return if zero flag is set.
int CPU::RET_Z()
{
	if (GET_ZERO_FLAG)
	{
		reg_PC.dat = (*mMap)[reg_SP.dat] | ((*mMap)[reg_SP.dat + 1] << 8);
		reg_SP.dat += 2;
		debugPrint("RET Z\n");
		return 20;
	}
	else
	{
		reg_PC.dat += 1;
		debugPrint("RET Z\n");
		return 8;
	}
}

// RET
// Return.
int CPU::RET()
{
	reg_PC.dat = (*mMap)[reg_SP.dat] | ((*mMap)[reg_SP.dat + 1] << 8);
	reg_SP.dat += 2;
	debugPrint("RET\n");
	return 16;
}

// JP Z, u16
// Jump to address u16 if zero flag is set.
int CPU::JP_Z_u16()
{
	if (GET_ZERO_FLAG)
	{
		reg_PC.dat = (*mMap)[reg_PC.dat + 1] | ((*mMap)[reg_PC.dat + 2] << 8);
		debugPrint("JP Z, %04X\n", reg_PC.dat);
		return 16;
	}
	else
	{
		reg_PC.dat += 3;
		debugPrint("JP Z, %04X\n", reg_PC.dat);
		return 12;
	}
}

// CB prefix
// Execute CB prefixed opcode.
int CPU::PREFIX_CB()
{
	reg_PC.dat += 1;
	int temp = executePrefixedInstruction();
	debugPrint("PREFIX CB\n");
	return temp + 4;
}

// CALL Z, u16
// Call subroutine at address u16 if zero flag is set.
int CPU::CALL_Z_u16()
{
	if (GET_ZERO_FLAG)
	{
		mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 3) >> 8);
		mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 3) & 0xFF);
		reg_PC.dat = (*mMap)[reg_PC.dat + 1] | ((*mMap)[reg_PC.dat + 2] << 8);
		debugPrint("CALL Z, %04X\n", reg_PC.dat);
		return 24;
	}
	else
	{
		reg_PC.dat += 3;
		debugPrint("CALL Z, %04X\n", reg_PC.dat);
		return 12;
	}
}

// CALL u16
// Call subroutine at address u16.
int CPU::CALL_u16()
{
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 3) >> 8);
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 3) & 0xFF);
	reg_PC.dat = (*mMap)[reg_PC.dat + 1] | ((*mMap)[reg_PC.dat + 2] << 8);
	debugPrint("CALL %04X\n", reg_PC.dat);
	return 24;
}

// ADC A, u8
// Add u8 + carry flag to A and set flags accordingly.
int CPU::ADC_A_u8()
{
	// Unset subtract flag
	UNSET_SUBTRACT_FLAG;

	Word temp = (Word)reg_AF.hi + GET_CARRY_FLAG + (*mMap)[reg_PC.dat + 1];

	// Set zero flag if A + u8 + carry flag == 0
	(reg_AF.hi + (*mMap)[reg_PC.dat + 1] + GET_CARRY_FLAG) & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	// Set half carry flag if lower nibble of A + lower nibble of u8 + carry flag > 0xF
	(reg_AF.hi & 0x0F) + ((*mMap)[reg_PC.dat + 1] & 0x0F) + GET_CARRY_FLAG > 0xF ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	0xFF < temp ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_AF.hi = temp;
	reg_PC.dat += 2;
	debugPrint("ADC A, %02X\n", (*mMap)[reg_PC.dat + 1]);
	return 8;
}

// RST 08H
// Call subroutine at address 0x0008.
int CPU::RST_08H()
{
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) >> 8);
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) & 0xFF);
	reg_PC.dat = 0x0008;
	debugPrint("RST 08H\n");
	return 16;
}

// RET NC
// Return if carry flag is not set.
int CPU::RET_NC()
{
	if (!GET_CARRY_FLAG)
	{
		reg_PC.dat = (*mMap)[reg_SP.dat] | ((*mMap)[reg_SP.dat + 1] << 8);
		reg_SP.dat += 2;
		debugPrint("RET NC\n");
		return 20;
	}
	else
	{
		reg_PC.dat += 1;
		debugPrint("RET NC\n");
		return 8;
	}
}

// POP DE
// Pop 16-bit value from stack into DE.
int CPU::POP_DE()
{
	reg_DE.dat = (*mMap)[reg_SP.dat] | ((*mMap)[reg_SP.dat + 1] << 8);
	reg_SP.dat += 2;
	reg_PC.dat += 1;
	debugPrint("POP DE\n");
	return 12;
}

// JP NC, u16
// Jump to address u16 if carry flag is not set.
int CPU::JP_NC_u16()
{
	if (!GET_CARRY_FLAG)
	{
		reg_PC.dat = (*mMap)[reg_PC.dat + 1] | ((*mMap)[reg_PC.dat + 2] << 8);
		debugPrint("JP NC, %04X\n", reg_PC.dat);
		return 16;
	}
	else
	{
		reg_PC.dat += 3;
		debugPrint("JP NC, %04X\n", reg_PC.dat);
		return 12;
	}
}

int CPU::UNKNOWN()
{
	const char* s = NULL;
	debugPrint("%c\n", s[0]);
	return 0;
}

// NCALL u16
// Call subroutine at address u16 if carry flag is not set.
int CPU::NC_u16()
{
	if (!GET_CARRY_FLAG)
	{
		mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 3) >> 8);
		mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 3) & 0xFF);
		reg_PC.dat = (*mMap)[reg_PC.dat + 1] | ((*mMap)[reg_PC.dat + 2] << 8);
		debugPrint("NCALL %04X\n", reg_PC.dat);
		return 24;
	}
	else
	{
		reg_PC.dat += 3;
		debugPrint("NCALL %04X\n", reg_PC.dat);
		return 12;
	}
}

// PUSH DE
// Push 16-bit value from DE onto stack.
int CPU::PUSH_DE()
{
	mMap->writeMemory(--reg_SP.dat, reg_DE.hi);
	mMap->writeMemory(--reg_SP.dat, reg_DE.lo);
	reg_PC.dat += 1;
	debugPrint("PUSH DE\n");
	return 16;
}

// SUB u8
// Subtract u8 from A and set flags accordingly.
int CPU::SUB_u8()
{
	// Set subtract flag
	SET_SUBTRACT_FLAG;

	// Set carry flag if A < u8
	reg_AF.hi < (*mMap)[reg_PC.dat + 1] ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A == u8
	reg_AF.hi == (*mMap)[reg_PC.dat + 1] ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;

	// Set half carry flag if lower nibble of A < lower nibble of u8
	(reg_AF.hi & 0x0F) < ((*mMap)[reg_PC.dat + 1] & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_AF.hi -= (*mMap)[reg_PC.dat + 1];

	reg_PC.dat += 2;
	debugPrint("SUB %02X\n", (*mMap)[reg_PC.dat - 1]);
	return 8;
}

// RST 10H
// Call subroutine at address 0x0010.
int CPU::RST_10H()
{
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) >> 8);
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) & 0xFF);
	reg_PC.dat = 0x0010;
	debugPrint("RST 10H\n");
	return 16;
}

// RET C
// Return if carry flag is set.
int CPU::RET_C()
{
	if (GET_CARRY_FLAG)
	{
		reg_PC.dat = (*mMap)[reg_SP.dat] | ((*mMap)[reg_SP.dat + 1] << 8);
		reg_SP.dat += 2;
		debugPrint("RET C\n");
		return 20;
	}
	else
	{
		reg_PC.dat += 1;
		debugPrint("RET C\n");
		return 8;
	}
}

// RETI
// Return and enable interrupts.
int CPU::RETI()
{
	reg_PC.dat = (*mMap)[reg_SP.dat] | ((*mMap)[reg_SP.dat + 1] << 8);
	reg_SP.dat += 2;
	// Instantly enable interrupts
	// as RETI is basically EI then RET
	// So 1 opcode delay of EI is taken care of
	IMEFlag = 1;
	IMEReg = true;

	debugPrint("RETI\n");
	return 16;
}

// JP C, u16
// Jump to address u16 if carry flag is set.
int CPU::JP_C_u16()
{
	if (GET_CARRY_FLAG)
	{
		reg_PC.dat = (*mMap)[reg_PC.dat + 1] | ((*mMap)[reg_PC.dat + 2] << 8);
		debugPrint("JP C, %04X\n", reg_PC.dat);
		return 16;
	}
	else
	{
		reg_PC.dat += 3;
		debugPrint("JP C, %04X\n", reg_PC.dat);
		return 12;
	}
}

// CALL C, u16
// Call subroutine at address u16 if carry flag is set.
int CPU::CALL_C_u16()
{
	if (GET_CARRY_FLAG)
	{
		mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 3) >> 8);
		mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 3) & 0xFF);
		reg_PC.dat = (*mMap)[reg_PC.dat + 1] | ((*mMap)[reg_PC.dat + 2] << 8);
		debugPrint("CALL C, %04X\n", reg_PC.dat);
		return 24;
	}
	else
	{
		reg_PC.dat += 3;
		debugPrint("CALL C, %04X\n", reg_PC.dat);
		return 12;
	}
}

// SBC A, u8
// Subtract u8 + carry flag from A and set flags accordingly.
int CPU::SBC_A_u8()
{
	// Set subtract flag
	SET_SUBTRACT_FLAG;

	Byte temp = reg_AF.hi;

	// Set half carry flag if lower nibble of A < lower nibble of u8 + carry flag
	(reg_AF.hi & 0x0F) < ((*mMap)[reg_PC.dat + 1] & 0x0F) + GET_CARRY_FLAG ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	reg_AF.hi -= ((*mMap)[reg_PC.dat + 1] + GET_CARRY_FLAG);

	// Set carry flag if A < u8 + carry flag
	temp < ((*mMap)[reg_PC.dat + 1] + GET_CARRY_FLAG) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A == u8 + carry flag
	reg_AF.hi == 0 ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;

	reg_PC.dat += 2;
	debugPrint("SBC A, %02X\n", (*mMap)[reg_PC.dat + 1]);
	return 8;
}

// RST 18H
// Call subroutine at address 0x0018.
int CPU::RST_18H()
{
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) >> 8);
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) & 0xFF);
	reg_PC.dat = 0x0018;
	debugPrint("RST 18H\n");
	return 16;
}
// LD (FF00+u8),A
// Load A into (0xFF00 + a8)
int CPU::LDH_a8_A()
{
	mMap->writeMemory(0xFF00 + (*mMap)[reg_PC.dat + 1], reg_AF.hi);
	reg_PC.dat += 2;
	debugPrint("LDH (%02X), A\n", (*mMap)[reg_PC.dat + 1]);
	return 12;
}

// POP HL
// Pop 16-bit value from stack into HL.
int CPU::POP_HL()
{
	reg_HL.dat = (*mMap)[reg_SP.dat] | ((*mMap)[reg_SP.dat + 1] << 8);
	reg_SP.dat += 2;
	reg_PC.dat += 1;
	debugPrint("POP HL\n");
	return 12;
}

// LD (FF00+C),A
// Load A into (0xFF00 + C)
int CPU::LDH_C_A()
{
	mMap->writeMemory(0xFF00 + reg_BC.lo, reg_AF.hi);
	reg_PC.dat += 1;
	debugPrint("LD (C), A\n");
	return 8;
}

// PUSH HL
// Push HL onto stack.
int CPU::PUSH_HL()
{
	mMap->writeMemory(--reg_SP.dat, reg_HL.hi);
	mMap->writeMemory(--reg_SP.dat, reg_HL.lo);
	reg_PC.dat += 1;
	debugPrint("PUSH HL\n");
	return 16;
}

//
int CPU::AND_A_u8()
{
	reg_AF.hi &= (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;

	// Set flags
	SET_HALF_CARRY_FLAG;
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;
	reg_AF.hi == 0 ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;
	return 8;
}

// RST 20H
// Call subroutine at address 0x0020.
int CPU::RST_20H()
{
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) >> 8);
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) & 0xFF);
	reg_PC.dat = 0x0020;
	debugPrint("RST 20H\n");
	return 16;
}

// ADD SP, i8
// Add i8 to SP and set flags accordingly.
int CPU::ADD_SP_i8()
{
	UNSET_ZERO_FLAG;
	UNSET_SUBTRACT_FLAG;

	// Set half carry flag if overflowed 3rd bit
	((reg_SP.dat & 0x0F) + ((SByte)(*mMap)[reg_PC.dat + 1] & 0x0F)) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if overflowed 7th bit
	((reg_SP.dat & 0xFF) + ((SByte)(*mMap)[reg_PC.dat + 1] & 0xFF)) & 0x100 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_SP.dat += (SByte)(*mMap)[reg_PC.dat + 1];

	reg_PC.dat += 2;
	debugPrint("ADD SP, i8\n");
	return 16;
}

// JP (HL)
// Jump to address contained in HL.
int CPU::JP_HL()
{
	reg_PC.dat = reg_HL.dat;
	return 4;
}

// LD (u16), A
// Load A into (u16)
int CPU::LD_u16_A()
{
	// u16 is ((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2]
	// Writing the value of A into the (u16)
	mMap->writeMemory((*mMap)[reg_PC.dat + 2] << 8 | (*mMap)[reg_PC.dat + 1], reg_AF.hi);
	reg_PC.dat += 3;
	debugPrint("LD (u16), A\n");
	return 16;
}

// XOR A, u8
// XOR A with u8 and set flags accordingly.
int CPU::XOR_A_u8()
{
	// Unset subtract and carry flags
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	// Unset half carry flag
	UNSET_HALF_CARRY_FLAG;

	reg_AF.hi ^= (*mMap)[reg_PC.dat + 1];

	// Set zero flag if A == u8
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 2;
	debugPrint("XOR A, %02X\n", (*mMap)[reg_PC.dat + 1]);
	return 8;
}

// RST 28H
// Call subroutine at address 0x0028.
int CPU::RST_28H()
{
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) >> 8);
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) & 0xFF);
	reg_PC.dat = 0x0028;
	debugPrint("RST 28H\n");
	return 16;
}

// LD A, (FF00+u8)
// Load (0xFF00 + a8) into A
int CPU::LDH_A_a8()
{
	reg_AF.hi = (*mMap)[0xFF00 + (*mMap)[reg_PC.dat + 1]];
	reg_PC.dat += 2;
	debugPrint("LD A, (FF00+%02X)\n", (*mMap)[reg_PC.dat + 1]);
	return 12;
}

// POP AF
// Pop 16-bit value from stack into AF.
int CPU::POP_AF()
{
	reg_AF.dat = (*mMap)[reg_SP.dat] & 0xF0 | ((*mMap)[reg_SP.dat + 1] << 8);
	reg_SP.dat += 2;
	reg_PC.dat += 1;
	debugPrint("POP AF\n");
	return 12;
}

// LDH A, (C)
// Load (0xFF00 + C) into A
int CPU::LDH_A_C()
{
	reg_AF.hi = (*mMap)[0xFF00 + reg_BC.lo];
	reg_PC.dat += 1;
	return 8;
}

// DI
// Disable interrupts
// TODO: Implement interrupts
int CPU::DI()
{
	// Set IMEFlag to -1 and immediately set IMEReg to false
	IMEFlag = -1;
	IMEReg = false;
	reg_PC.dat += 1;
	debugPrint("DI\n");
	return 4;
}

// PUSH AF
// Push AF onto stack.
int CPU::PUSH_AF()
{
	mMap->writeMemory(--reg_SP.dat, reg_AF.hi);
	mMap->writeMemory(--reg_SP.dat, reg_AF.lo);
	reg_PC.dat += 1;
	debugPrint("PUSH AF\n");
	return 16;
}

// OR A, u8
// OR A with u8 and set flags accordingly.
int CPU::OR_A_u8()
{
	// Unset subtract and carry flags
	UNSET_SUBTRACT_FLAG;
	UNSET_CARRY_FLAG;

	// Unset half carry flag
	UNSET_HALF_CARRY_FLAG;

	reg_AF.hi |= (*mMap)[reg_PC.dat + 1];

	// Set zero flag if A == u8
	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 2;
	debugPrint("OR A, %02X\n", (*mMap)[reg_PC.dat + 1]);
	return 8;
}

// RST 30H
// Call subroutine at address 0x0030.
int CPU::RST_30H()
{
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) >> 8);
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) & 0xFF);
	reg_PC.dat = 0x0030;
	debugPrint("RST 30H\n");
	return 16;
}

// LD HL, SP + i8
// Load SP + i8 into HL
int CPU::LD_HL_SP_i8()
{
	UNSET_ZERO_FLAG;
	UNSET_SUBTRACT_FLAG;

	// Set half carry flag if overflowed 3rd bit
	((reg_SP.dat & 0x0F) + ((SByte)(*mMap)[reg_PC.dat + 1] & 0x0F)) & 0x10 ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if overflowed 7th bit
	((reg_SP.dat & 0xFF) + ((SByte)(*mMap)[reg_PC.dat + 1] & 0xFF)) & 0x100 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	reg_HL.dat = reg_SP.dat + (SByte)(*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	return 12;
}

// LD SP, HL
// Load HL into SP
int CPU::LD_SP_HL()
{
	reg_SP.dat = reg_HL.dat;
	reg_PC.dat += 1;
	return 8;
}

// LD A, (u16)
// Load (u16) into A
int CPU::LD_A_u16()
{
	reg_AF.hi = (*mMap)[((*mMap)[reg_PC.dat + 2] << 8) | (*mMap)[reg_PC.dat + 1]];
	reg_PC.dat += 3;
	debugPrint("LD A, (HL)\n");
	return 16;
}

// EI
// Enable interrupts
// TODO: Implement interrupts
int CPU::EI()
{
	// Check the comments on the definition of IMEFlag and IMEReg
	IMEFlag = 0;
	reg_PC.dat += 1;
	debugPrint("EI\n");
	return 4;
}

// CP A, u8
// Compare A with u8 and set flags accordingly.
int CPU::CP_u8()
{
	// Set subtract flag
	SET_SUBTRACT_FLAG;

	// Set half carry flag if lower nibble of A is less than lower nibble of u8
	(reg_AF.hi & 0x0F) < ((*mMap)[reg_PC.dat + 1] & 0x0F) ? SET_HALF_CARRY_FLAG : UNSET_HALF_CARRY_FLAG;

	// Set carry flag if A is less than u8
	reg_AF.hi < (*mMap)[reg_PC.dat + 1] ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Set zero flag if A == u8
	reg_AF.hi == (*mMap)[reg_PC.dat + 1] ? SET_ZERO_FLAG : UNSET_ZERO_FLAG;

	reg_PC.dat += 2;
	debugPrint("CP A, %02X\n", (*mMap)[reg_PC.dat + 1]);
	return 8;
}

// RST 38H
// Call subroutine at address 0x0038.
int CPU::RST_38H()
{
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) >> 8);
	mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) & 0xFF);
	reg_PC.dat = 0x0038;
	debugPrint("RST 38H\n");
	return 16;
}

int CPU::executeInstruction(Byte opcode)
{
	return (this->*method_pointer[opcode])();
}

int CPU::executeNextInstruction()
{
	// Check if boot execution is complete
	// If yes, we can do logging in debug log outfile
	if (mMap->readMemory(0xFF50) == 0x01)
	{
		dumpState();
	}

	// Turn off logging
	// If reached infinite loop
	if (reg_PC.dat == 0xCC62)
	{
		fclose(outfile);
	}

	// Get the opcode
	Byte opcode = (*mMap)[reg_PC.dat];
	return (this->*method_pointer[opcode])();
}

int CPU::executePrefixedInstruction()
{
	// Get the opcode
	Byte opcode = (*mMap)[reg_PC.dat];
	return (this->*prefixed_method_pointer[opcode])();
}

// RLC B
// Rotate B left
int CPU::RLC_B()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_BC.hi >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate B left by 1
	reg_BC.hi = (reg_BC.hi << 1) | (reg_BC.hi >> 7);

	reg_BC.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RLC B\n");
	return 4;
}

// RLC C
// Rotate C left
int CPU::RLC_C()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_BC.lo >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate C left by 1
	reg_BC.lo = (reg_BC.lo << 1) | (reg_BC.lo >> 7);

	reg_BC.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RLC C\n");
	return 4;
}

// RLC D
// Rotate D left
int CPU::RLC_D()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_DE.hi >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate D left by 1
	reg_DE.hi = (reg_DE.hi << 1) | (reg_DE.hi >> 7);

	reg_DE.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RLC D\n");
	return 4;
}

// RLC E
// Rotate E left
int CPU::RLC_E()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_DE.lo >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate E left by 1
	reg_DE.lo = (reg_DE.lo << 1) | (reg_DE.lo >> 7);

	reg_DE.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RLC E\n");
	return 4;
}

// RLC H
// Rotate H left
int CPU::RLC_H()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_HL.hi >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate H left by 1
	reg_HL.hi = (reg_HL.hi << 1) | (reg_HL.hi >> 7);

	reg_HL.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RLC H\n");
	return 4;
}

// RLC L
// Rotate L left
int CPU::RLC_L()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_HL.lo >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate L left by 1
	reg_HL.lo = (reg_HL.lo << 1) | (reg_HL.lo >> 7);

	reg_HL.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RLC L\n");
	return 4;
}

// RLC (HL)
// Rotate the value at memory address pointed to by HL left
int CPU::RLC_HLp()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	(*mMap)[reg_HL.dat] >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate the value at memory address pointed to by HL left by 1
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] << 1) | ((*mMap)[reg_HL.dat] >> 7));

	(*mMap)[reg_HL.dat] ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RLC (HL)\n");
	return 4;
}

// RLC A
// Rotate A left
int CPU::RLC_A()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_AF.hi >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate A left by 1
	reg_AF.hi = (reg_AF.hi << 1) | (reg_AF.hi >> 7);

	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RLC A\n");
	return 4;
}

// RRC B
// Rotate B right
int CPU::RRC_B()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_BC.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate B right by 1
	reg_BC.hi = (reg_BC.hi >> 1) | (reg_BC.hi << 7);

	reg_BC.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RRC B\n");
	return 4;
}

// RRC C
// Rotate C right
int CPU::RRC_C()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_BC.lo & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate C right by 1
	reg_BC.lo = (reg_BC.lo >> 1) | (reg_BC.lo << 7);

	reg_BC.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RRC C\n");
	return 4;
}

// RRC D
// Rotate D right
int CPU::RRC_D()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_DE.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate D right by 1
	reg_DE.hi = (reg_DE.hi >> 1) | (reg_DE.hi << 7);

	reg_DE.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RRC D\n");
	return 4;
}

// RRC E
// Rotate E right
int CPU::RRC_E()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_DE.lo & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate E right by 1
	reg_DE.lo = (reg_DE.lo >> 1) | (reg_DE.lo << 7);

	reg_DE.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RRC E\n");
	return 4;
}

// RRC H
// Rotate H right
int CPU::RRC_H()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_HL.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate H right by 1
	reg_HL.hi = (reg_HL.hi >> 1) | (reg_HL.hi << 7);

	reg_HL.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RRC H\n");
	return 4;
}

// RRC L
// Rotate L right
int CPU::RRC_L()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_HL.lo & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate L right by 1
	reg_HL.lo = (reg_HL.lo >> 1) | (reg_HL.lo << 7);

	reg_HL.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RRC L\n");
	return 4;
}

// RRC (HL)
// Rotate the value at memory address pointed to by HL right
int CPU::RRC_HLp()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	(*mMap)[reg_HL.dat] & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate the value at memory address pointed to by HL right by 1
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] >> 1) | ((*mMap)[reg_HL.dat] << 7));

	(*mMap)[reg_HL.dat] ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RRC (HL)\n");
	return 4;
}

// RRC A
// Rotate A right
int CPU::RRC_A()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_AF.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Rotate A right by 1
	reg_AF.hi = (reg_AF.hi >> 1) | (reg_AF.hi << 7);

	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RRC A\n");
	return 4;
}

// RL B
// Rotate B left and load the carry flag into its 0th bit
int CPU::RL_B()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate B left by 1
	reg_BC.hi = (reg_BC.hi << 1) | (reg_BC.hi >> 7);

	// swap the values of 0th bit of B and the carry flag
	reg_BC.hi ^= GET_CARRY_FLAG;
	GET_CARRY_FLAG ^ (reg_BC.hi & 1) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_BC.hi ^= GET_CARRY_FLAG;

	reg_BC.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RL B\n");
	return 4;
}

// RL C
// Rotate C left and load the carry flag into its 0th bit
int CPU::RL_C()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate C left by 1
	reg_BC.lo = (reg_BC.lo << 1) | (reg_BC.lo >> 7);

	// swap the values of 0th bit of C and the carry flag
	reg_BC.lo ^= GET_CARRY_FLAG;
	GET_CARRY_FLAG ^ (reg_BC.lo & 1) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_BC.lo ^= GET_CARRY_FLAG;

	reg_BC.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RL C\n");
	return 4;
}

// RL D
// Rotate D left and load the carry flag into its 0th bit
int CPU::RL_D()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate D left by 1
	reg_DE.hi = (reg_DE.hi << 1) | (reg_DE.hi >> 7);

	// swap the values of 0th bit of D and the carry flag
	reg_DE.hi ^= GET_CARRY_FLAG;
	GET_CARRY_FLAG ^ (reg_DE.hi & 1) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_DE.hi ^= GET_CARRY_FLAG;

	reg_DE.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RL D\n");
	return 4;
}

// RL E
// Rotate E left and load the carry flag into its 0th bit
int CPU::RL_E()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate E left by 1
	reg_DE.lo = (reg_DE.lo << 1) | (reg_DE.lo >> 7);

	// swap the values of 0th bit of E and the carry flag
	reg_DE.lo ^= GET_CARRY_FLAG;
	GET_CARRY_FLAG ^ (reg_DE.lo & 1) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_DE.lo ^= GET_CARRY_FLAG;

	reg_DE.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RL E\n");
	return 4;
}

// RL H
// Rotate H left and load the carry flag into its 0th bit
int CPU::RL_H()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate H left by 1
	reg_HL.hi = (reg_HL.hi << 1) | (reg_HL.hi >> 7);

	// swap the values of 0th bit of H and the carry flag
	reg_HL.hi ^= GET_CARRY_FLAG;
	GET_CARRY_FLAG ^ (reg_HL.hi & 1) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_HL.hi ^= GET_CARRY_FLAG;

	reg_HL.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RL H\n");
	return 4;
}

// RL L
// Rotate L left and load the carry flag into its 0th bit
int CPU::RL_L()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate L left by 1
	reg_HL.lo = (reg_HL.lo << 1) | (reg_HL.lo >> 7);

	// swap the values of 0th bit of L and the carry flag
	reg_HL.lo ^= GET_CARRY_FLAG;
	GET_CARRY_FLAG ^ (reg_HL.lo & 1) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_HL.lo ^= GET_CARRY_FLAG;

	reg_HL.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RL L\n");
	return 4;
}

// RL (HL)
// Rotate the value at memory address pointed to by HL left and load the carry flag into its 0th bit
int CPU::RL_HLp()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate the value at memory address pointed to by HL left by 1
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] << 1) | ((*mMap)[reg_HL.dat] >> 7));

	// swap the values of 0th bit of the value at memory address pointed to by HL and the carry flag
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] ^ GET_CARRY_FLAG);
	GET_CARRY_FLAG ^ ((*mMap)[reg_HL.dat] & 1) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] ^ GET_CARRY_FLAG);

	(*mMap)[reg_HL.dat] ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RL (HL)\n");
	return 4;
}

// RL A
// Rotate A left and load the carry flag into its 0th bit
int CPU::RL_A()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate A left by 1
	reg_AF.hi = (reg_AF.hi << 1) | (reg_AF.hi >> 7);

	// swap the values of 0th bit of A and the carry flag
	reg_AF.hi ^= GET_CARRY_FLAG;
	GET_CARRY_FLAG ^ (reg_AF.hi & 1) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_AF.hi ^= GET_CARRY_FLAG;

	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RL A\n");
	return 4;
}

// RR B
// Rotate B right and load the carry flag into its 7th bit
int CPU::RR_B()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate B right by 1
	reg_BC.hi = (reg_BC.hi >> 1) | (reg_BC.hi << 7);

	// swap the values of 7th bit of B and the carry flag
	reg_BC.hi ^= GET_CARRY_FLAG << 7;
	GET_CARRY_FLAG ^ (reg_BC.hi >> 7) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_BC.hi ^= GET_CARRY_FLAG << 7;

	reg_BC.hi & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RR B\n");
	return 4;
}

// RR C
// Rotate C right and load the carry flag into its 7th bit
int CPU::RR_C()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate C right by 1
	reg_BC.lo = (reg_BC.lo >> 1) | (reg_BC.lo << 7);

	// swap the values of 7th bit of C and the carry flag
	reg_BC.lo ^= GET_CARRY_FLAG << 7;
	GET_CARRY_FLAG ^ (reg_BC.lo >> 7) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_BC.lo ^= GET_CARRY_FLAG << 7;

	reg_BC.lo & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RR C\n");
	return 4;
}

// RR D
// Rotate D right and load the carry flag into its 7th bit
int CPU::RR_D()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate D right by 1
	reg_DE.hi = (reg_DE.hi >> 1) | (reg_DE.hi << 7);

	// swap the values of 7th bit of D and the carry flag
	reg_DE.hi ^= GET_CARRY_FLAG << 7;
	GET_CARRY_FLAG ^ (reg_DE.hi >> 7) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_DE.hi ^= GET_CARRY_FLAG << 7;

	reg_DE.hi & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RR D\n");
	return 4;
}

// RR E
// Rotate E right and load the carry flag into its 7th bit
int CPU::RR_E()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate E right by 1
	reg_DE.lo = (reg_DE.lo >> 1) | (reg_DE.lo << 7);

	// swap the values of 7th bit of E and the carry flag
	reg_DE.lo ^= GET_CARRY_FLAG << 7;
	GET_CARRY_FLAG ^ (reg_DE.lo >> 7) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_DE.lo ^= GET_CARRY_FLAG << 7;

	reg_DE.lo & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RR E\n");
	return 4;
}

// RR H
// Rotate H right and load the carry flag into its 7th bit
int CPU::RR_H()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate H right by 1
	reg_HL.hi = (reg_HL.hi >> 1) | (reg_HL.hi << 7);

	// swap the values of 7th bit of H and the carry flag
	reg_HL.hi ^= GET_CARRY_FLAG << 7;
	GET_CARRY_FLAG ^ (reg_HL.hi >> 7) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_HL.hi ^= GET_CARRY_FLAG << 7;

	reg_HL.hi & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RR H\n");
	return 4;
}

// RR L
// Rotate L right and load the carry flag into its 7th bit
int CPU::RR_L()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate L right by 1
	reg_HL.lo = (reg_HL.lo >> 1) | (reg_HL.lo << 7);

	// swap the values of 7th bit of L and the carry flag
	reg_HL.lo ^= GET_CARRY_FLAG << 7;
	GET_CARRY_FLAG ^ (reg_HL.lo >> 7) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_HL.lo ^= GET_CARRY_FLAG << 7;

	reg_HL.lo & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RR L\n");
	return 4;
}

// RR (HL)
// Rotate the value at memory address pointed to by HL right and load the carry flag into its 7th bit
int CPU::RR_HLp()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate the value at memory address pointed to by HL right by 1
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] >> 1) | ((*mMap)[reg_HL.dat] << 7));

	// swap the values of 7th bit of the value at memory address pointed to by HL and the carry flag
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] ^ GET_CARRY_FLAG << 7);
	GET_CARRY_FLAG ^ ((*mMap)[reg_HL.dat] >> 7) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] ^ GET_CARRY_FLAG << 7);

	(*mMap)[reg_HL.dat] & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RR (HL)\n");
	return 4;
}

// RR A
// Rotate A right and load the carry flag into its 7th bit
int CPU::RR_A()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// Rotate A right by 1
	reg_AF.hi = (reg_AF.hi >> 1) | (reg_AF.hi << 7);

	// swap the values of 7th bit of A and the carry flag
	reg_AF.hi ^= GET_CARRY_FLAG << 7;
	GET_CARRY_FLAG ^ (reg_AF.hi >> 7) ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;
	reg_AF.hi ^= GET_CARRY_FLAG << 7;

	reg_AF.hi & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("RR A\n");
	return 4;
}

// SLA B
// Shift B left
int CPU::SLA_B()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_BC.hi >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift B left by 1
	reg_BC.hi = (reg_BC.hi << 1);

	reg_BC.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SLA B\n");
	return 4;
}

// SLA C
// Shift C left
int CPU::SLA_C()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_BC.lo >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift C left by 1
	reg_BC.lo = (reg_BC.lo << 1);

	reg_BC.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SLA C\n");
	return 4;
}

// SLA D
// Shift D left
int CPU::SLA_D()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_DE.hi >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift D left by 1
	reg_DE.hi = (reg_DE.hi << 1);

	reg_DE.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SLA D\n");
	return 4;
}

// SLA E
// Shift E left
int CPU::SLA_E()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_DE.lo >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift E left by 1
	reg_DE.lo = (reg_DE.lo << 1);

	reg_DE.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SLA E\n");
	return 4;
}

// SLA H
// Shift H left
int CPU::SLA_H()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_HL.hi >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift H left by 1
	reg_HL.hi = (reg_HL.hi << 1);

	reg_HL.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SLA H\n");
	return 4;
}

// SLA L
// Shift L left
int CPU::SLA_L()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_HL.lo >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift L left by 1
	reg_HL.lo = (reg_HL.lo << 1);

	reg_HL.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SLA L\n");
	return 4;
}

// SLA (HL)
// Shift the value at memory address pointed to by HL left
int CPU::SLA_HLp()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	(*mMap)[reg_HL.dat] >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift the value at memory address pointed to by HL left by 1
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] << 1));

	(*mMap)[reg_HL.dat] ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SLA (HL)\n");
	return 4;
}

// SLA A
// Shift A left
int CPU::SLA_A()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 7 in carry flag
	reg_AF.hi >> 7 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift A left by 1
	reg_AF.hi = (reg_AF.hi << 1);

	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SLA A\n");
	return 4;
}

// SRA B
// Shift B right while leaving 7th bit unchanged
int CPU::SRA_B()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_BC.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift B right by 1 while leaving 7th bit unchanged
	reg_BC.hi = (reg_BC.hi >> 1) | (reg_BC.hi & 0x80);

	reg_BC.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRA B\n");
	return 4;
}

// SRA C
// Shift C right while leaving 7th bit unchanged
int CPU::SRA_C()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_BC.lo & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift C right by 1 while leaving 7th bit unchanged
	reg_BC.lo = (reg_BC.lo >> 1) | (reg_BC.lo & 0x80);

	reg_BC.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRA C\n");
	return 4;
}

// SRA D
// Shift D right while leaving 7th bit unchanged
int CPU::SRA_D()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_DE.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift D right by 1 while leaving 7th bit unchanged
	reg_DE.hi = (reg_DE.hi >> 1) | (reg_DE.hi & 0x80);

	reg_DE.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRA D\n");
	return 4;
}

// SRA E
// Shift E right while leaving 7th bit unchanged
int CPU::SRA_E()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_DE.lo & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift E right by 1 while leaving 7th bit unchanged
	reg_DE.lo = (reg_DE.lo >> 1) | (reg_DE.lo & 0x80);

	reg_DE.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRA E\n");
	return 4;
}

// SRA H
// Shift H right while leaving 7th bit unchanged
int CPU::SRA_H()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_HL.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift H right by 1 while leaving 7th bit unchanged
	reg_HL.hi = (reg_HL.hi >> 1) | (reg_HL.hi & 0x80);

	reg_HL.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRA H\n");
	return 4;
}

// SRA L
// Shift L right while leaving 7th bit unchanged
int CPU::SRA_L()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_HL.lo & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift L right by 1 while leaving 7th bit unchanged
	reg_HL.lo = (reg_HL.lo >> 1) | (reg_HL.lo & 0x80);

	reg_HL.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRA L\n");
	return 4;
}

// SRA (HL)
// Shift the value at memory address pointed to by HL right while leaving 7th bit unchanged
int CPU::SRA_HLp()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	(*mMap)[reg_HL.dat] & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift the value at memory address pointed to by HL right by 1 while leaving 7th bit unchanged
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] >> 1) | ((*mMap)[reg_HL.dat] & 0x80));

	(*mMap)[reg_HL.dat] ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRA (HL)\n");
	return 4;
}

// SRA A
// Shift A right while leaving 7th bit unchanged
int CPU::SRA_A()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_AF.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift A right by 1 while leaving 7th bit unchanged
	reg_AF.hi = (reg_AF.hi >> 1) | (reg_AF.hi & 0x80);

	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRA A\n");
	return 4;
}

// SWAP B
// Swap the upper and lower nibbles of B
int CPU::SWAP_B()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;
	UNSET_CARRY_FLAG;

	// swap the upper and lower nibbles
	reg_BC.hi = (((reg_BC.hi & 0x0F) << 4) | ((reg_BC.hi & 0xF0) >> 4));

	reg_BC.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SWAP B\n");
	return 4;
}

// SWAP C
// Swap the upper and lower nibbles of C
int CPU::SWAP_C()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;
	UNSET_CARRY_FLAG;

	// swap the upper and lower nibbles
	reg_BC.lo = (((reg_BC.lo & 0x0F) << 4) | ((reg_BC.lo & 0xF0) >> 4));

	reg_BC.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SWAP C\n");
	return 4;
}

// SWAP D
// Swap the upper and lower nibbles of D
int CPU::SWAP_D()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;
	UNSET_CARRY_FLAG;

	// swap the upper and lower nibbles
	reg_DE.hi = (((reg_DE.hi & 0x0F) << 4) | ((reg_DE.hi & 0xF0) >> 4));

	reg_DE.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SWAP D\n");
	return 4;
}

// SWAP E
// Swap the upper and lower nibbles of E
int CPU::SWAP_E()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;
	UNSET_CARRY_FLAG;

	// swap the upper and lower nibbles
	reg_DE.lo = (((reg_DE.lo & 0x0F) << 4) | ((reg_DE.lo & 0xF0) >> 4));

	reg_DE.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SWAP E\n");
	return 4;
}

// SWAP H
// Swap the upper and lower nibbles of H
int CPU::SWAP_H()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;
	UNSET_CARRY_FLAG;

	// swap the upper and lower nibbles
	reg_HL.hi = (((reg_HL.hi & 0x0F) << 4) | ((reg_HL.hi & 0xF0) >> 4));

	reg_HL.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SWAP H\n");
	return 4;
}

// SWAP L
// Swap the upper and lower nibbles of L
int CPU::SWAP_L()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;
	UNSET_CARRY_FLAG;

	// swap the upper and lower nibbles
	reg_HL.lo = (((reg_HL.lo & 0x0F) << 4) | ((reg_HL.lo & 0xF0) >> 4));

	reg_HL.lo ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SWAP L\n");
	return 4;
}

// SWAP (HL)
// Swap the upper and lower nibbles of the value at memory address pointed to by HL
int CPU::SWAP_HLp()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;
	UNSET_CARRY_FLAG;

	// swap the upper and lower nibbles
	mMap->writeMemory(reg_HL.dat, ((((*mMap)[reg_HL.dat] & 0x0F) << 4) | (((*mMap)[reg_HL.dat] & 0xF0) >> 4)));

	(*mMap)[reg_HL.dat] ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SWAP (HL)\n");
	return 4;
}

// SWAP A
// Swap the upper and lower nibbles of A
int CPU::SWAP_A()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;
	UNSET_CARRY_FLAG;

	// swap the upper and lower nibbles
	reg_AF.hi = (((reg_AF.hi & 0x0F) << 4) | ((reg_AF.hi & 0xF0) >> 4));

	reg_AF.hi ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SWAP A\n");
	return 4;
}

// SRL B
// Shift B right
int CPU::SRL_B()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_BC.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift B right by 1
	reg_BC.hi = (reg_BC.hi >> 1);

	reg_BC.hi & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRL B\n");
	return 4;
}

// SRL C
// Shift C right
int CPU::SRL_C()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_BC.lo & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift C right by 1
	reg_BC.lo = (reg_BC.lo >> 1);

	reg_BC.lo & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRL C\n");
	return 4;
}

// SRL D
// Shift D right
int CPU::SRL_D()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_DE.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift D right by 1
	reg_DE.hi = (reg_DE.hi >> 1);

	reg_DE.hi & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRL D\n");
	return 4;
}

// SRL E
// Shift E right
int CPU::SRL_E()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_DE.lo & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift E right by 1
	reg_DE.lo = (reg_DE.lo >> 1);

	reg_DE.lo & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRL E\n");
	return 4;
}

// SRL H
// Shift H right
int CPU::SRL_H()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_HL.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift H right by 1
	reg_HL.hi = (reg_HL.hi >> 1);

	reg_HL.hi & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRL H\n");
	return 4;
}

// SRL L
// Shift L right
int CPU::SRL_L()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_HL.lo & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift L right by 1
	reg_HL.lo = (reg_HL.lo >> 1);

	reg_HL.lo & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRL L\n");
	return 4;
}

// SRL (HL)
// Shift the value at memory address pointed to by HL right
int CPU::SRL_HLp()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	(*mMap)[reg_HL.dat] & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift the value at memory address pointed to by HL right by 1
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] >> 1));

	(*mMap)[reg_HL.dat] & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRL (HL)\n");
	return 4;
}

// SRL A
// Shift A right
int CPU::SRL_A()
{
	UNSET_SUBTRACT_FLAG;
	UNSET_HALF_CARRY_FLAG;

	// store bit 0 in carry flag
	reg_AF.hi & 1 ? SET_CARRY_FLAG : UNSET_CARRY_FLAG;

	// Shift A right by 1
	reg_AF.hi = (reg_AF.hi >> 1);

	reg_AF.hi & 0xFF ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("SRL A\n");
	return 4;
}

// BIT 0, B
// Stores the compliment of 0th bit of B in zero flag
int CPU::BIT_0_B()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 0th bit in zero flag
	(reg_BC.hi & (1 << 0)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 0, B\n");
	return 4;
}

// BIT 0, C
// Stores the compliment of 0th bit of C in zero flag
int CPU::BIT_0_C()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 0th bit in zero flag
	(reg_BC.lo & (1 << 0)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 0, C\n");
	return 4;
}

// BIT 0, D
// Stores the compliment of 0th bit of D in zero flag
int CPU::BIT_0_D()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 0th bit in zero flag
	(reg_DE.hi & (1 << 0)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 0, D\n");
	return 4;
}

// BIT 0, E
// Stores the compliment of 0th bit of E in zero flag
int CPU::BIT_0_E()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 0th bit in zero flag
	(reg_DE.lo & (1 << 0)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 0, E\n");
	return 4;
}

// BIT 0, H
// Stores the compliment of 0th bit of H in zero flag
int CPU::BIT_0_H()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 0th bit in zero flag
	(reg_HL.hi & (1 << 0)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 0, H\n");
	return 4;
}

// BIT 0, L
// Stores the compliment of 0th bit of L in zero flag
int CPU::BIT_0_L()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 0th bit in zero flag
	(reg_HL.lo & (1 << 0)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 0, L\n");
	return 4;
}

// BIT 0, (HL)
// Stores the compliment of 0th bit of the value at memory address pointed to by HL in zero flag
int CPU::BIT_0_HLp()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 0th bit in zero flag
	((*mMap)[reg_HL.dat] & (1 << 0)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 0, (HL)\n");
	return 4;
}

// BIT 0, A
// Stores the compliment of 0th bit of A in zero flag
int CPU::BIT_0_A()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 0th bit in zero flag
	(reg_AF.hi & (1 << 0)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 0, A\n");
	return 4;
}

// BIT 1, B
// Stores the compliment of 1st bit of B in zero flag
int CPU::BIT_1_B()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 1st bit in zero flag
	(reg_BC.hi & (1 << 1)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 1, B\n");
	return 4;
}

// BIT 1, C
// Stores the compliment of 1st bit of C in zero flag
int CPU::BIT_1_C()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 1st bit in zero flag
	(reg_BC.lo & (1 << 1)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 1, C\n");
	return 4;
}

// BIT 1, D
// Stores the compliment of 1st bit of D in zero flag
int CPU::BIT_1_D()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 1st bit in zero flag
	(reg_DE.hi & (1 << 1)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 1, D\n");
	return 4;
}

// BIT 1, E
// Stores the compliment of 1st bit of E in zero flag
int CPU::BIT_1_E()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 1st bit in zero flag
	(reg_DE.lo & (1 << 1)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 1, E\n");
	return 4;
}

// BIT 1, H
// Stores the compliment of 1st bit of H in zero flag
int CPU::BIT_1_H()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 1st bit in zero flag
	(reg_HL.hi & (1 << 1)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 1, H\n");
	return 4;
}

// BIT 1, L
// Stores the compliment of 1st bit of L in zero flag
int CPU::BIT_1_L()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 1st bit in zero flag
	(reg_HL.lo & (1 << 1)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 1, L\n");
	return 4;
}

// BIT 1, (HL)
// Stores the compliment of 1st bit of the value at memory address pointed to by HL in zero flag
int CPU::BIT_1_HLp()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 1st bit in zero flag
	((*mMap)[reg_HL.dat] & (1 << 1)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 1, (HL)\n");
	return 4;
}

// BIT 1, A
// Stores the compliment of 1st bit of A in zero flag
int CPU::BIT_1_A()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 1st bit in zero flag
	(reg_AF.hi & (1 << 1)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 1, A\n");
	return 4;
}

// BIT 2, B
// Stores the compliment of 2nd bit of B in zero flag
int CPU::BIT_2_B()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 2nd bit in zero flag
	(reg_BC.hi & (1 << 2)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 2, B\n");
	return 4;
}

// BIT 2, C
// Stores the compliment of 2nd bit of C in zero flag
int CPU::BIT_2_C()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 2nd bit in zero flag
	(reg_BC.lo & (1 << 2)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 2, C\n");
	return 4;
}

// BIT 2, D
// Stores the compliment of 2nd bit of D in zero flag
int CPU::BIT_2_D()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 2nd bit in zero flag
	(reg_DE.hi & (1 << 2)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 2, D\n");
	return 4;
}

// BIT 2, E
// Stores the compliment of 2nd bit of E in zero flag
int CPU::BIT_2_E()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 2nd bit in zero flag
	(reg_DE.lo & (1 << 2)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 2, E\n");
	return 4;
}

// BIT 2, H
// Stores the compliment of 2nd bit of H in zero flag
int CPU::BIT_2_H()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 2nd bit in zero flag
	(reg_HL.hi & (1 << 2)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 2, H\n");
	return 4;
}

// BIT 2, L
// Stores the compliment of 2nd bit of L in zero flag
int CPU::BIT_2_L()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 2nd bit in zero flag
	(reg_HL.lo & (1 << 2)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 2, L\n");
	return 4;
}

// BIT 2, (HL)
// Stores the compliment of 2nd bit of the value at memory address pointed to by HL in zero flag
int CPU::BIT_2_HLp()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 2nd bit in zero flag
	((*mMap)[reg_HL.dat] & (1 << 2)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 2, (HL)\n");
	return 4;
}

// BIT 2, A
// Stores the compliment of 2nd bit of A in zero flag
int CPU::BIT_2_A()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 2nd bit in zero flag
	(reg_AF.hi & (1 << 2)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 2, A\n");
	return 4;
}

// BIT 3, B
// Stores the compliment of 3rd bit of B in zero flag
int CPU::BIT_3_B()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 3rd bit in zero flag
	(reg_BC.hi & (1 << 3)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 3, B\n");
	return 4;
}

// BIT 3, C
// Stores the compliment of 3rd bit of C in zero flag
int CPU::BIT_3_C()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 3rd bit in zero flag
	(reg_BC.lo & (1 << 3)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 3, C\n");
	return 4;
}

// BIT 3, D
// Stores the compliment of 3rd bit of D in zero flag
int CPU::BIT_3_D()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 3rd bit in zero flag
	(reg_DE.hi & (1 << 3)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 3, D\n");
	return 4;
}

// BIT 3, E
// Stores the compliment of 3rd bit of E in zero flag
int CPU::BIT_3_E()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 3rd bit in zero flag
	(reg_DE.lo & (1 << 3)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 3, E\n");
	return 4;
}

// BIT 3, H
// Stores the compliment of 3rd bit of H in zero flag
int CPU::BIT_3_H()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 3rd bit in zero flag
	(reg_HL.hi & (1 << 3)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 3, H\n");
	return 4;
}

// BIT 3, L
// Stores the compliment of 3rd bit of L in zero flag
int CPU::BIT_3_L()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 3rd bit in zero flag
	(reg_HL.lo & (1 << 3)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 3, L\n");
	return 4;
}

// BIT 3, (HL)
// Stores the compliment of 3rd bit of the value at memory address pointed to by HL in zero flag
int CPU::BIT_3_HLp()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 3rd bit in zero flag
	((*mMap)[reg_HL.dat] & (1 << 3)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 3, (HL)\n");
	return 4;
}

// BIT 3, A
// Stores the compliment of 3rd bit of A in zero flag
int CPU::BIT_3_A()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 3rd bit in zero flag
	(reg_AF.hi & (1 << 3)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 3, A\n");
	return 4;
}

// BIT 4, B
// Stores the compliment of 4th bit of B in zero flag
int CPU::BIT_4_B()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 4th bit in zero flag
	(reg_BC.hi & (1 << 4)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 4, B\n");
	return 4;
}

// BIT 4, C
// Stores the compliment of 4th bit of C in zero flag
int CPU::BIT_4_C()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 4th bit in zero flag
	(reg_BC.lo & (1 << 4)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 4, C\n");
	return 4;
}

// BIT 4, D
// Stores the compliment of 4th bit of D in zero flag
int CPU::BIT_4_D()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 4th bit in zero flag
	(reg_DE.hi & (1 << 4)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 4, D\n");
	return 4;
}

// BIT 4, E
// Stores the compliment of 4th bit of E in zero flag
int CPU::BIT_4_E()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 4th bit in zero flag
	(reg_DE.lo & (1 << 4)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 4, E\n");
	return 4;
}

// BIT 4, H
// Stores the compliment of 4th bit of H in zero flag
int CPU::BIT_4_H()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 4th bit in zero flag
	(reg_HL.hi & (1 << 4)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 4, H\n");
	return 4;
}

// BIT 4, L
// Stores the compliment of 4th bit of L in zero flag
int CPU::BIT_4_L()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 4th bit in zero flag
	(reg_HL.lo & (1 << 4)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 4, L\n");
	return 4;
}

// BIT 4, (HL)
// Stores the compliment of 4th bit of the value at memory address pointed to by HL in zero flag
int CPU::BIT_4_HLp()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 4th bit in zero flag
	((*mMap)[reg_HL.dat] & (1 << 4)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 4, (HL)\n");
	return 4;
}

// BIT 4, A
// Stores the compliment of 4th bit of A in zero flag
int CPU::BIT_4_A()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 4th bit in zero flag
	(reg_AF.hi & (1 << 4)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 4, A\n");
	return 4;
}

// BIT 5, B
// Stores the compliment of 5th bit of B in zero flag
int CPU::BIT_5_B()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 5th bit in zero flag
	(reg_BC.hi & (1 << 5)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 5, B\n");
	return 4;
}

// BIT 5, C
// Stores the compliment of 5th bit of C in zero flag
int CPU::BIT_5_C()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 5th bit in zero flag
	(reg_BC.lo & (1 << 5)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 5, C\n");
	return 4;
}

// BIT 5, D
// Stores the compliment of 5th bit of D in zero flag
int CPU::BIT_5_D()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 5th bit in zero flag
	(reg_DE.hi & (1 << 5)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 5, D\n");
	return 4;
}

// BIT 5, E
// Stores the compliment of 5th bit of E in zero flag
int CPU::BIT_5_E()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 5th bit in zero flag
	(reg_DE.lo & (1 << 5)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 5, E\n");
	return 4;
}

// BIT 5, H
// Stores the compliment of 5th bit of H in zero flag
int CPU::BIT_5_H()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 5th bit in zero flag
	(reg_HL.hi & (1 << 5)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 5, H\n");
	return 4;
}

// BIT 5, L
// Stores the compliment of 5th bit of L in zero flag
int CPU::BIT_5_L()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 5th bit in zero flag
	(reg_HL.lo & (1 << 5)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 5, L\n");
	return 4;
}

// BIT 5, (HL)
// Stores the compliment of 5th bit of the value at memory address pointed to by HL in zero flag
int CPU::BIT_5_HLp()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 5th bit in zero flag
	((*mMap)[reg_HL.dat] & (1 << 5)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 5, (HL)\n");
	return 4;
}

// BIT 5, A
// Stores the compliment of 5th bit of A in zero flag
int CPU::BIT_5_A()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 5th bit in zero flag
	(reg_AF.hi & (1 << 5)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 5, A\n");
	return 4;
}

// BIT 6, B
// Stores the compliment of 6th bit of B in zero flag
int CPU::BIT_6_B()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 6th bit in zero flag
	(reg_BC.hi & (1 << 6)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 6, B\n");
	return 4;
}

// BIT 6, C
// Stores the compliment of 6th bit of C in zero flag
int CPU::BIT_6_C()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 6th bit in zero flag
	(reg_BC.lo & (1 << 6)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 6, C\n");
	return 4;
}

// BIT 6, D
// Stores the compliment of 6th bit of D in zero flag
int CPU::BIT_6_D()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 6th bit in zero flag
	(reg_DE.hi & (1 << 6)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 6, D\n");
	return 4;
}

// BIT 6, E
// Stores the compliment of 6th bit of E in zero flag
int CPU::BIT_6_E()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 6th bit in zero flag
	(reg_DE.lo & (1 << 6)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 6, E\n");
	return 4;
}

// BIT 6, H
// Stores the compliment of 6th bit of H in zero flag
int CPU::BIT_6_H()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 6th bit in zero flag
	(reg_HL.hi & (1 << 6)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 6, H\n");
	return 4;
}

// BIT 6, L
// Stores the compliment of 6th bit of L in zero flag
int CPU::BIT_6_L()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 6th bit in zero flag
	(reg_HL.lo & (1 << 6)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 6, L\n");
	return 4;
}

// BIT 6, (HL)
// Stores the compliment of 6th bit of the value at memory address pointed to by HL in zero flag
int CPU::BIT_6_HLp()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 6th bit in zero flag
	((*mMap)[reg_HL.dat] & (1 << 6)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 6, (HL)\n");
	return 4;
}

// BIT 6, A
// Stores the compliment of 6th bit of A in zero flag
int CPU::BIT_6_A()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 6th bit in zero flag
	(reg_AF.hi & (1 << 6)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 6, A\n");
	return 4;
}

// BIT 7, B
// Stores the compliment of 7th bit of B in zero flag
int CPU::BIT_7_B()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 7th bit in zero flag
	(reg_BC.hi & (1 << 7)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 7, B\n");
	return 4;
}

// BIT 7, C
// Stores the compliment of 7th bit of C in zero flag
int CPU::BIT_7_C()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 7th bit in zero flag
	(reg_BC.lo & (1 << 7)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 7, C\n");
	return 4;
}

// BIT 7, D
// Stores the compliment of 7th bit of D in zero flag
int CPU::BIT_7_D()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 7th bit in zero flag
	(reg_DE.hi & (1 << 7)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 7, D\n");
	return 4;
}

// BIT 7, E
// Stores the compliment of 7th bit of E in zero flag
int CPU::BIT_7_E()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 7th bit in zero flag
	(reg_DE.lo & (1 << 7)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 7, E\n");
	return 4;
}

// BIT 7, H
// Stores the compliment of 7th bit of H in zero flag
int CPU::BIT_7_H()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 7th bit in zero flag
	(reg_HL.hi & (1 << 7)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 7, H\n");
	return 4;
}

// BIT 7, L
// Stores the compliment of 7th bit of L in zero flag
int CPU::BIT_7_L()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 7th bit in zero flag
	(reg_HL.lo & (1 << 7)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 7, L\n");
	return 4;
}

// BIT 7, (HL)
// Stores the compliment of 7th bit of the value at memory address pointed to by HL in zero flag
int CPU::BIT_7_HLp()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 7th bit in zero flag
	((*mMap)[reg_HL.dat] & (1 << 7)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 7, (HL)\n");
	return 4;
}

// BIT 7, A
// Stores the compliment of 7th bit of A in zero flag
int CPU::BIT_7_A()
{
	UNSET_SUBTRACT_FLAG;
	SET_HALF_CARRY_FLAG;

	// store the compliment of 7th bit in zero flag
	(reg_AF.hi & (1 << 7)) ? UNSET_ZERO_FLAG : SET_ZERO_FLAG;

	reg_PC.dat += 1;
	debugPrint("BIT 7, A\n");
	return 4;
}

// RES 0, B
// Unsets the 0th bit of B
int CPU::RES_0_B()
{
	// unset the 0th bit
	reg_BC.hi &= 0xFF ^ (1 << 0);

	reg_PC.dat += 1;
	debugPrint("RES 0, B\n");
	return 4;
}

// RES 0, C
// Unsets the 0th bit of C
int CPU::RES_0_C()
{
	// unset the 0th bit
	reg_BC.lo &= 0xFF ^ (1 << 0);

	reg_PC.dat += 1;
	debugPrint("RES 0, C\n");
	return 4;
}

// RES 0, D
// Unsets the 0th bit of D
int CPU::RES_0_D()
{
	// unset the 0th bit
	reg_DE.hi &= 0xFF ^ (1 << 0);

	reg_PC.dat += 1;
	debugPrint("RES 0, D\n");
	return 4;
}

// RES 0, E
// Unsets the 0th bit of E
int CPU::RES_0_E()
{
	// unset the 0th bit
	reg_DE.lo &= 0xFF ^ (1 << 0);

	reg_PC.dat += 1;
	debugPrint("RES 0, E\n");
	return 4;
}

// RES 0, H
// Unsets the 0th bit of H
int CPU::RES_0_H()
{
	// unset the 0th bit
	reg_HL.hi &= 0xFF ^ (1 << 0);

	reg_PC.dat += 1;
	debugPrint("RES 0, H\n");
	return 4;
}

// RES 0, L
// Unsets the 0th bit of L
int CPU::RES_0_L()
{
	// unset the 0th bit
	reg_HL.lo &= 0xFF ^ (1 << 0);

	reg_PC.dat += 1;
	debugPrint("RES 0, L\n");
	return 4;
}

// RES 0, (HL)
// Unsets the 0th bit of the value at memory address pointed to by HL
int CPU::RES_0_HLp()
{
	// unset the 0th bit
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] & (0xFF ^ (1 << 0))));

	reg_PC.dat += 1;
	debugPrint("RES 0, (HL)\n");
	return 4;
}

// RES 0, A
// Unsets the 0th bit of A
int CPU::RES_0_A()
{
	// unset the 0th bit
	reg_AF.hi &= 0xFF ^ (1 << 0);

	reg_PC.dat += 1;
	debugPrint("RES 0, A\n");
	return 4;
}

// RES 1, B
// Unsets the 1st bit of B
int CPU::RES_1_B()
{
	// unset the 1st bit
	reg_BC.hi &= 0xFF ^ (1 << 1);

	reg_PC.dat += 1;
	debugPrint("RES 1, B\n");
	return 4;
}

// RES 1, C
// Unsets the 1st bit of C
int CPU::RES_1_C()
{
	// unset the 1st bit
	reg_BC.lo &= 0xFF ^ (1 << 1);

	reg_PC.dat += 1;
	debugPrint("RES 1, C\n");
	return 4;
}

// RES 1, D
// Unsets the 1st bit of D
int CPU::RES_1_D()
{
	// unset the 1st bit
	reg_DE.hi &= 0xFF ^ (1 << 1);

	reg_PC.dat += 1;
	debugPrint("RES 1, D\n");
	return 4;
}

// RES 1, E
// Unsets the 1st bit of E
int CPU::RES_1_E()
{
	// unset the 1st bit
	reg_DE.lo &= 0xFF ^ (1 << 1);

	reg_PC.dat += 1;
	debugPrint("RES 1, E\n");
	return 4;
}

// RES 1, H
// Unsets the 1st bit of H
int CPU::RES_1_H()
{
	// unset the 1st bit
	reg_HL.hi &= 0xFF ^ (1 << 1);

	reg_PC.dat += 1;
	debugPrint("RES 1, H\n");
	return 4;
}

// RES 1, L
// Unsets the 1st bit of L
int CPU::RES_1_L()
{
	// unset the 1st bit
	reg_HL.lo &= 0xFF ^ (1 << 1);

	reg_PC.dat += 1;
	debugPrint("RES 1, L\n");
	return 4;
}

// RES 1, (HL)
// Unsets the 1st bit of the value at memory address pointed to by HL
int CPU::RES_1_HLp()
{
	// unset the 1st bit
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] & (0xFF ^ (1 << 1))));

	reg_PC.dat += 1;
	debugPrint("RES 1, (HL)\n");
	return 4;
}

// RES 1, A
// Unsets the 1st bit of A
int CPU::RES_1_A()
{
	// unset the 1st bit
	reg_AF.hi &= 0xFF ^ (1 << 1);

	reg_PC.dat += 1;
	debugPrint("RES 1, A\n");
	return 4;
}

// RES 2, B
// Unsets the 2nd bit of B
int CPU::RES_2_B()
{
	// unset the 2nd bit
	reg_BC.hi &= 0xFF ^ (1 << 2);

	reg_PC.dat += 1;
	debugPrint("RES 2, B\n");
	return 4;
}

// RES 2, C
// Unsets the 2nd bit of C
int CPU::RES_2_C()
{
	// unset the 2nd bit
	reg_BC.lo &= 0xFF ^ (1 << 2);

	reg_PC.dat += 1;
	debugPrint("RES 2, C\n");
	return 4;
}

// RES 2, D
// Unsets the 2nd bit of D
int CPU::RES_2_D()
{
	// unset the 2nd bit
	reg_DE.hi &= 0xFF ^ (1 << 2);

	reg_PC.dat += 1;
	debugPrint("RES 2, D\n");
	return 4;
}

// RES 2, E
// Unsets the 2nd bit of E
int CPU::RES_2_E()
{
	// unset the 2nd bit
	reg_DE.lo &= 0xFF ^ (1 << 2);

	reg_PC.dat += 1;
	debugPrint("RES 2, E\n");
	return 4;
}

// RES 2, H
// Unsets the 2nd bit of H
int CPU::RES_2_H()
{
	// unset the 2nd bit
	reg_HL.hi &= 0xFF ^ (1 << 2);

	reg_PC.dat += 1;
	debugPrint("RES 2, H\n");
	return 4;
}

// RES 2, L
// Unsets the 2nd bit of L
int CPU::RES_2_L()
{
	// unset the 2nd bit
	reg_HL.lo &= 0xFF ^ (1 << 2);

	reg_PC.dat += 1;
	debugPrint("RES 2, L\n");
	return 4;
}

// RES 2, (HL)
// Unsets the 2nd bit of the value at memory address pointed to by HL
int CPU::RES_2_HLp()
{
	// unset the 2nd bit
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] & (0xFF ^ (1 << 2))));

	reg_PC.dat += 1;
	debugPrint("RES 2, (HL)\n");
	return 4;
}

// RES 2, A
// Unsets the 2nd bit of A
int CPU::RES_2_A()
{
	// unset the 2nd bit
	reg_AF.hi &= 0xFF ^ (1 << 2);

	reg_PC.dat += 1;
	debugPrint("RES 2, A\n");
	return 4;
}

// RES 3, B
// Unsets the 3rd bit of B
int CPU::RES_3_B()
{
	// unset the 3rd bit
	reg_BC.hi &= 0xFF ^ (1 << 3);

	reg_PC.dat += 1;
	debugPrint("RES 3, B\n");
	return 4;
}

// RES 3, C
// Unsets the 3rd bit of C
int CPU::RES_3_C()
{
	// unset the 3rd bit
	reg_BC.lo &= 0xFF ^ (1 << 3);

	reg_PC.dat += 1;
	debugPrint("RES 3, C\n");
	return 4;
}

// RES 3, D
// Unsets the 3rd bit of D
int CPU::RES_3_D()
{
	// unset the 3rd bit
	reg_DE.hi &= 0xFF ^ (1 << 3);

	reg_PC.dat += 1;
	debugPrint("RES 3, D\n");
	return 4;
}

// RES 3, E
// Unsets the 3rd bit of E
int CPU::RES_3_E()
{
	// unset the 3rd bit
	reg_DE.lo &= 0xFF ^ (1 << 3);

	reg_PC.dat += 1;
	debugPrint("RES 3, E\n");
	return 4;
}

// RES 3, H
// Unsets the 3rd bit of H
int CPU::RES_3_H()
{
	// unset the 3rd bit
	reg_HL.hi &= 0xFF ^ (1 << 3);

	reg_PC.dat += 1;
	debugPrint("RES 3, H\n");
	return 4;
}

// RES 3, L
// Unsets the 3rd bit of L
int CPU::RES_3_L()
{
	// unset the 3rd bit
	reg_HL.lo &= 0xFF ^ (1 << 3);

	reg_PC.dat += 1;
	debugPrint("RES 3, L\n");
	return 4;
}

// RES 3, (HL)
// Unsets the 3rd bit of the value at memory address pointed to by HL
int CPU::RES_3_HLp()
{
	// unset the 3rd bit
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] & (0xFF ^ (1 << 3))));

	reg_PC.dat += 1;
	debugPrint("RES 3, (HL)\n");
	return 4;
}

// RES 3, A
// Unsets the 3rd bit of A
int CPU::RES_3_A()
{
	// unset the 3rd bit
	reg_AF.hi &= 0xFF ^ (1 << 3);

	reg_PC.dat += 1;
	debugPrint("RES 3, A\n");
	return 4;
}

// RES 4, B
// Unsets the 4th bit of B
int CPU::RES_4_B()
{
	// unset the 4th bit
	reg_BC.hi &= 0xFF ^ (1 << 4);

	reg_PC.dat += 1;
	debugPrint("RES 4, B\n");
	return 4;
}

// RES 4, C
// Unsets the 4th bit of C
int CPU::RES_4_C()
{
	// unset the 4th bit
	reg_BC.lo &= 0xFF ^ (1 << 4);

	reg_PC.dat += 1;
	debugPrint("RES 4, C\n");
	return 4;
}

// RES 4, D
// Unsets the 4th bit of D
int CPU::RES_4_D()
{
	// unset the 4th bit
	reg_DE.hi &= 0xFF ^ (1 << 4);

	reg_PC.dat += 1;
	debugPrint("RES 4, D\n");
	return 4;
}

// RES 4, E
// Unsets the 4th bit of E
int CPU::RES_4_E()
{
	// unset the 4th bit
	reg_DE.lo &= 0xFF ^ (1 << 4);

	reg_PC.dat += 1;
	debugPrint("RES 4, E\n");
	return 4;
}

// RES 4, H
// Unsets the 4th bit of H
int CPU::RES_4_H()
{
	// unset the 4th bit
	reg_HL.hi &= 0xFF ^ (1 << 4);

	reg_PC.dat += 1;
	debugPrint("RES 4, H\n");
	return 4;
}

// RES 4, L
// Unsets the 4th bit of L
int CPU::RES_4_L()
{
	// unset the 4th bit
	reg_HL.lo &= 0xFF ^ (1 << 4);

	reg_PC.dat += 1;
	debugPrint("RES 4, L\n");
	return 4;
}

// RES 4, (HL)
// Unsets the 4th bit of the value at memory address pointed to by HL
int CPU::RES_4_HLp()
{
	// unset the 4th bit
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] & (0xFF ^ (1 << 4))));

	reg_PC.dat += 1;
	debugPrint("RES 4, (HL)\n");
	return 4;
}

// RES 4, A
// Unsets the 4th bit of A
int CPU::RES_4_A()
{
	// unset the 4th bit
	reg_AF.hi &= 0xFF ^ (1 << 4);

	reg_PC.dat += 1;
	debugPrint("RES 4, A\n");
	return 4;
}

// RES 5, B
// Unsets the 5th bit of B
int CPU::RES_5_B()
{
	// unset the 5th bit
	reg_BC.hi &= 0xFF ^ (1 << 5);

	reg_PC.dat += 1;
	debugPrint("RES 5, B\n");
	return 4;
}

// RES 5, C
// Unsets the 5th bit of C
int CPU::RES_5_C()
{
	// unset the 5th bit
	reg_BC.lo &= 0xFF ^ (1 << 5);

	reg_PC.dat += 1;
	debugPrint("RES 5, C\n");
	return 4;
}

// RES 5, D
// Unsets the 5th bit of D
int CPU::RES_5_D()
{
	// unset the 5th bit
	reg_DE.hi &= 0xFF ^ (1 << 5);

	reg_PC.dat += 1;
	debugPrint("RES 5, D\n");
	return 4;
}

// RES 5, E
// Unsets the 5th bit of E
int CPU::RES_5_E()
{
	// unset the 5th bit
	reg_DE.lo &= 0xFF ^ (1 << 5);

	reg_PC.dat += 1;
	debugPrint("RES 5, E\n");
	return 4;
}

// RES 5, H
// Unsets the 5th bit of H
int CPU::RES_5_H()
{
	// unset the 5th bit
	reg_HL.hi &= 0xFF ^ (1 << 5);

	reg_PC.dat += 1;
	debugPrint("RES 5, H\n");
	return 4;
}

// RES 5, L
// Unsets the 5th bit of L
int CPU::RES_5_L()
{
	// unset the 5th bit
	reg_HL.lo &= 0xFF ^ (1 << 5);

	reg_PC.dat += 1;
	debugPrint("RES 5, L\n");
	return 4;
}

// RES 5, (HL)
// Unsets the 5th bit of the value at memory address pointed to by HL
int CPU::RES_5_HLp()
{
	// unset the 5th bit
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] & (0xFF ^ (1 << 5))));

	reg_PC.dat += 1;
	debugPrint("RES 5, (HL)\n");
	return 4;
}

// RES 5, A
// Unsets the 5th bit of A
int CPU::RES_5_A()
{
	// unset the 5th bit
	reg_AF.hi &= 0xFF ^ (1 << 5);

	reg_PC.dat += 1;
	debugPrint("RES 5, A\n");
	return 4;
}

// RES 6, B
// Unsets the 6th bit of B
int CPU::RES_6_B()
{
	// unset the 6th bit
	reg_BC.hi &= 0xFF ^ (1 << 6);

	reg_PC.dat += 1;
	debugPrint("RES 6, B\n");
	return 4;
}

// RES 6, C
// Unsets the 6th bit of C
int CPU::RES_6_C()
{
	// unset the 6th bit
	reg_BC.lo &= 0xFF ^ (1 << 6);

	reg_PC.dat += 1;
	debugPrint("RES 6, C\n");
	return 4;
}

// RES 6, D
// Unsets the 6th bit of D
int CPU::RES_6_D()
{
	// unset the 6th bit
	reg_DE.hi &= 0xFF ^ (1 << 6);

	reg_PC.dat += 1;
	debugPrint("RES 6, D\n");
	return 4;
}

// RES 6, E
// Unsets the 6th bit of E
int CPU::RES_6_E()
{
	// unset the 6th bit
	reg_DE.lo &= 0xFF ^ (1 << 6);

	reg_PC.dat += 1;
	debugPrint("RES 6, E\n");
	return 4;
}

// RES 6, H
// Unsets the 6th bit of H
int CPU::RES_6_H()
{
	// unset the 6th bit
	reg_HL.hi &= 0xFF ^ (1 << 6);

	reg_PC.dat += 1;
	debugPrint("RES 6, H\n");
	return 4;
}

// RES 6, L
// Unsets the 6th bit of L
int CPU::RES_6_L()
{
	// unset the 6th bit
	reg_HL.lo &= 0xFF ^ (1 << 6);

	reg_PC.dat += 1;
	debugPrint("RES 6, L\n");
	return 4;
}

// RES 6, (HL)
// Unsets the 6th bit of the value at memory address pointed to by HL
int CPU::RES_6_HLp()
{
	// unset the 6th bit
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] & (0xFF ^ (1 << 6))));

	reg_PC.dat += 1;
	debugPrint("RES 6, (HL)\n");
	return 4;
}

// RES 6, A
// Unsets the 6th bit of A
int CPU::RES_6_A()
{
	// unset the 6th bit
	reg_AF.hi &= 0xFF ^ (1 << 6);

	reg_PC.dat += 1;
	debugPrint("RES 6, A\n");
	return 4;
}

// RES 7, B
// Unsets the 7th bit of B
int CPU::RES_7_B()
{
	// unset the 7th bit
	reg_BC.hi &= 0xFF ^ (1 << 7);

	reg_PC.dat += 1;
	debugPrint("RES 7, B\n");
	return 4;
}

// RES 7, C
// Unsets the 7th bit of C
int CPU::RES_7_C()
{
	// unset the 7th bit
	reg_BC.lo &= 0xFF ^ (1 << 7);

	reg_PC.dat += 1;
	debugPrint("RES 7, C\n");
	return 4;
}

// RES 7, D
// Unsets the 7th bit of D
int CPU::RES_7_D()
{
	// unset the 7th bit
	reg_DE.hi &= 0xFF ^ (1 << 7);

	reg_PC.dat += 1;
	debugPrint("RES 7, D\n");
	return 4;
}

// RES 7, E
// Unsets the 7th bit of E
int CPU::RES_7_E()
{
	// unset the 7th bit
	reg_DE.lo &= 0xFF ^ (1 << 7);

	reg_PC.dat += 1;
	debugPrint("RES 7, E\n");
	return 4;
}

// RES 7, H
// Unsets the 7th bit of H
int CPU::RES_7_H()
{
	// unset the 7th bit
	reg_HL.hi &= 0xFF ^ (1 << 7);

	reg_PC.dat += 1;
	debugPrint("RES 7, H\n");
	return 4;
}

// RES 7, L
// Unsets the 7th bit of L
int CPU::RES_7_L()
{
	// unset the 7th bit
	reg_HL.lo &= 0xFF ^ (1 << 7);

	reg_PC.dat += 1;
	debugPrint("RES 7, L\n");
	return 4;
}

// RES 7, (HL)
// Unsets the 7th bit of the value at memory address pointed to by HL
int CPU::RES_7_HLp()
{
	// unset the 7th bit
	mMap->writeMemory(reg_HL.dat, ((*mMap)[reg_HL.dat] & (0xFF ^ (1 << 7))));

	reg_PC.dat += 1;
	debugPrint("RES 7, (HL)\n");
	return 4;
}

// RES 7, A
// Unsets the 7th bit of A
int CPU::RES_7_A()
{
	// unset the 7th bit
	reg_AF.hi &= 0xFF ^ (1 << 7);

	reg_PC.dat += 1;
	debugPrint("RES 7, A\n");
	return 4;
}

// SET 0, B
// Sets the 0th bit of B
int CPU::SET_0_B()
{
	// set the 0th bit
	reg_BC.hi |= (1 << 0);

	reg_PC.dat += 1;
	debugPrint("SET 0, B\n");
	return 4;
}

// SET 0, C
// Sets the 0th bit of C
int CPU::SET_0_C()
{
	// set the 0th bit
	reg_BC.lo |= (1 << 0);

	reg_PC.dat += 1;
	debugPrint("SET 0, C\n");
	return 4;
}

// SET 0, D
// Sets the 0th bit of D
int CPU::SET_0_D()
{
	// set the 0th bit
	reg_DE.hi |= (1 << 0);

	reg_PC.dat += 1;
	debugPrint("SET 0, D\n");
	return 4;
}

// SET 0, E
// Sets the 0th bit of E
int CPU::SET_0_E()
{
	// set the 0th bit
	reg_DE.lo |= (1 << 0);

	reg_PC.dat += 1;
	debugPrint("SET 0, E\n");
	return 4;
}

// SET 0, H
// Sets the 0th bit of H
int CPU::SET_0_H()
{
	// set the 0th bit
	reg_HL.hi |= (1 << 0);

	reg_PC.dat += 1;
	debugPrint("SET 0, H\n");
	return 4;
}

// SET 0, L
// Sets the 0th bit of L
int CPU::SET_0_L()
{
	// set the 0th bit
	reg_HL.lo |= (1 << 0);

	reg_PC.dat += 1;
	debugPrint("SET 0, L\n");
	return 4;
}

// SET 0, (HL)
// Sets the 0th bit of the value at memory address pointed to by HL
int CPU::SET_0_HLp()
{
	// set the 0th bit
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] | (1 << 0));

	reg_PC.dat += 1;
	debugPrint("SET 0, (HL)\n");
	return 4;
}

// SET 0, A
// Sets the 0th bit of A
int CPU::SET_0_A()
{
	// set the 0th bit
	reg_AF.hi |= (1 << 0);

	reg_PC.dat += 1;
	debugPrint("SET 0, A\n");
	return 4;
}

// SET 1, B
// Sets the 1st bit of B
int CPU::SET_1_B()
{
	// set the 1st bit
	reg_BC.hi |= (1 << 1);

	reg_PC.dat += 1;
	debugPrint("SET 1, B\n");
	return 4;
}

// SET 1, C
// Sets the 1st bit of C
int CPU::SET_1_C()
{
	// set the 1st bit
	reg_BC.lo |= (1 << 1);

	reg_PC.dat += 1;
	debugPrint("SET 1, C\n");
	return 4;
}

// SET 1, D
// Sets the 1st bit of D
int CPU::SET_1_D()
{
	// set the 1st bit
	reg_DE.hi |= (1 << 1);

	reg_PC.dat += 1;
	debugPrint("SET 1, D\n");
	return 4;
}

// SET 1, E
// Sets the 1st bit of E
int CPU::SET_1_E()
{
	// set the 1st bit
	reg_DE.lo |= (1 << 1);

	reg_PC.dat += 1;
	debugPrint("SET 1, E\n");
	return 4;
}

// SET 1, H
// Sets the 1st bit of H
int CPU::SET_1_H()
{
	// set the 1st bit
	reg_HL.hi |= (1 << 1);

	reg_PC.dat += 1;
	debugPrint("SET 1, H\n");
	return 4;
}

// SET 1, L
// Sets the 1st bit of L
int CPU::SET_1_L()
{
	// set the 1st bit
	reg_HL.lo |= (1 << 1);

	reg_PC.dat += 1;
	debugPrint("SET 1, L\n");
	return 4;
}

// SET 1, (HL)
// Sets the 1st bit of the value at memory address pointed to by HL
int CPU::SET_1_HLp()
{
	// set the 1st bit
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] | (1 << 1));

	reg_PC.dat += 1;
	debugPrint("SET 1, (HL)\n");
	return 4;
}

// SET 1, A
// Sets the 1st bit of A
int CPU::SET_1_A()
{
	// set the 1st bit
	reg_AF.hi |= (1 << 1);

	reg_PC.dat += 1;
	debugPrint("SET 1, A\n");
	return 4;
}

// SET 2, B
// Sets the 2nd bit of B
int CPU::SET_2_B()
{
	// set the 2nd bit
	reg_BC.hi |= (1 << 2);

	reg_PC.dat += 1;
	debugPrint("SET 2, B\n");
	return 4;
}

// SET 2, C
// Sets the 2nd bit of C
int CPU::SET_2_C()
{
	// set the 2nd bit
	reg_BC.lo |= (1 << 2);

	reg_PC.dat += 1;
	debugPrint("SET 2, C\n");
	return 4;
}

// SET 2, D
// Sets the 2nd bit of D
int CPU::SET_2_D()
{
	// set the 2nd bit
	reg_DE.hi |= (1 << 2);

	reg_PC.dat += 1;
	debugPrint("SET 2, D\n");
	return 4;
}

// SET 2, E
// Sets the 2nd bit of E
int CPU::SET_2_E()
{
	// set the 2nd bit
	reg_DE.lo |= (1 << 2);

	reg_PC.dat += 1;
	debugPrint("SET 2, E\n");
	return 4;
}

// SET 2, H
// Sets the 2nd bit of H
int CPU::SET_2_H()
{
	// set the 2nd bit
	reg_HL.hi |= (1 << 2);

	reg_PC.dat += 1;
	debugPrint("SET 2, H\n");
	return 4;
}

// SET 2, L
// Sets the 2nd bit of L
int CPU::SET_2_L()
{
	// set the 2nd bit
	reg_HL.lo |= (1 << 2);

	reg_PC.dat += 1;
	debugPrint("SET 2, L\n");
	return 4;
}

// SET 2, (HL)
// Sets the 2nd bit of the value at memory address pointed to by HL
int CPU::SET_2_HLp()
{
	// set the 2nd bit
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] | (1 << 2));

	reg_PC.dat += 1;
	debugPrint("SET 2, (HL)\n");
	return 4;
}

// SET 2, A
// Sets the 2nd bit of A
int CPU::SET_2_A()
{
	// set the 2nd bit
	reg_AF.hi |= (1 << 2);

	reg_PC.dat += 1;
	debugPrint("SET 2, A\n");
	return 4;
}

// SET 3, B
// Sets the 3rd bit of B
int CPU::SET_3_B()
{
	// set the 3rd bit
	reg_BC.hi |= (1 << 3);

	reg_PC.dat += 1;
	debugPrint("SET 3, B\n");
	return 4;
}

// SET 3, C
// Sets the 3rd bit of C
int CPU::SET_3_C()
{
	// set the 3rd bit
	reg_BC.lo |= (1 << 3);

	reg_PC.dat += 1;
	debugPrint("SET 3, C\n");
	return 4;
}

// SET 3, D
// Sets the 3rd bit of D
int CPU::SET_3_D()
{
	// set the 3rd bit
	reg_DE.hi |= (1 << 3);

	reg_PC.dat += 1;
	debugPrint("SET 3, D\n");
	return 4;
}

// SET 3, E
// Sets the 3rd bit of E
int CPU::SET_3_E()
{
	// set the 3rd bit
	reg_DE.lo |= (1 << 3);

	reg_PC.dat += 1;
	debugPrint("SET 3, E\n");
	return 4;
}

// SET 3, H
// Sets the 3rd bit of H
int CPU::SET_3_H()
{
	// set the 3rd bit
	reg_HL.hi |= (1 << 3);

	reg_PC.dat += 1;
	debugPrint("SET 3, H\n");
	return 4;
}

// SET 3, L
// Sets the 3rd bit of L
int CPU::SET_3_L()
{
	// set the 3rd bit
	reg_HL.lo |= (1 << 3);

	reg_PC.dat += 1;
	debugPrint("SET 3, L\n");
	return 4;
}

// SET 3, (HL)
// Sets the 3rd bit of the value at memory address pointed to by HL
int CPU::SET_3_HLp()
{
	// set the 3rd bit
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] | (1 << 3));

	reg_PC.dat += 1;
	debugPrint("SET 3, (HL)\n");
	return 4;
}

// SET 3, A
// Sets the 3rd bit of A
int CPU::SET_3_A()
{
	// set the 3rd bit
	reg_AF.hi |= (1 << 3);

	reg_PC.dat += 1;
	debugPrint("SET 3, A\n");
	return 4;
}

// SET 4, B
// Sets the 4th bit of B
int CPU::SET_4_B()
{
	// set the 4th bit
	reg_BC.hi |= (1 << 4);

	reg_PC.dat += 1;
	debugPrint("SET 4, B\n");
	return 4;
}

// SET 4, C
// Sets the 4th bit of C
int CPU::SET_4_C()
{
	// set the 4th bit
	reg_BC.lo |= (1 << 4);

	reg_PC.dat += 1;
	debugPrint("SET 4, C\n");
	return 4;
}

// SET 4, D
// Sets the 4th bit of D
int CPU::SET_4_D()
{
	// set the 4th bit
	reg_DE.hi |= (1 << 4);

	reg_PC.dat += 1;
	debugPrint("SET 4, D\n");
	return 4;
}

// SET 4, E
// Sets the 4th bit of E
int CPU::SET_4_E()
{
	// set the 4th bit
	reg_DE.lo |= (1 << 4);

	reg_PC.dat += 1;
	debugPrint("SET 4, E\n");
	return 4;
}

// SET 4, H
// Sets the 4th bit of H
int CPU::SET_4_H()
{
	// set the 4th bit
	reg_HL.hi |= (1 << 4);

	reg_PC.dat += 1;
	debugPrint("SET 4, H\n");
	return 4;
}

// SET 4, L
// Sets the 4th bit of L
int CPU::SET_4_L()
{
	// set the 4th bit
	reg_HL.lo |= (1 << 4);

	reg_PC.dat += 1;
	debugPrint("SET 4, L\n");
	return 4;
}

// SET 4, (HL)
// Sets the 4th bit of the value at memory address pointed to by HL
int CPU::SET_4_HLp()
{
	// set the 4th bit
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] | (1 << 4));

	reg_PC.dat += 1;
	debugPrint("SET 4, (HL)\n");
	return 4;
}

// SET 4, A
// Sets the 4th bit of A
int CPU::SET_4_A()
{
	// set the 4th bit
	reg_AF.hi |= (1 << 4);

	reg_PC.dat += 1;
	debugPrint("SET 4, A\n");
	return 4;
}

// SET 5, B
// Sets the 5th bit of B
int CPU::SET_5_B()
{
	// set the 5th bit
	reg_BC.hi |= (1 << 5);

	reg_PC.dat += 1;
	debugPrint("SET 5, B\n");
	return 4;
}

// SET 5, C
// Sets the 5th bit of C
int CPU::SET_5_C()
{
	// set the 5th bit
	reg_BC.lo |= (1 << 5);

	reg_PC.dat += 1;
	debugPrint("SET 5, C\n");
	return 4;
}

// SET 5, D
// Sets the 5th bit of D
int CPU::SET_5_D()
{
	// set the 5th bit
	reg_DE.hi |= (1 << 5);

	reg_PC.dat += 1;
	debugPrint("SET 5, D\n");
	return 4;
}

// SET 5, E
// Sets the 5th bit of E
int CPU::SET_5_E()
{
	// set the 5th bit
	reg_DE.lo |= (1 << 5);

	reg_PC.dat += 1;
	debugPrint("SET 5, E\n");
	return 4;
}

// SET 5, H
// Sets the 5th bit of H
int CPU::SET_5_H()
{
	// set the 5th bit
	reg_HL.hi |= (1 << 5);

	reg_PC.dat += 1;
	debugPrint("SET 5, H\n");
	return 4;
}

// SET 5, L
// Sets the 5th bit of L
int CPU::SET_5_L()
{
	// set the 5th bit
	reg_HL.lo |= (1 << 5);

	reg_PC.dat += 1;
	debugPrint("SET 5, L\n");
	return 4;
}

// SET 5, (HL)
// Sets the 5th bit of the value at memory address pointed to by HL
int CPU::SET_5_HLp()
{
	// set the 5th bit
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] | (1 << 5));

	reg_PC.dat += 1;
	debugPrint("SET 5, (HL)\n");
	return 4;
}

// SET 5, A
// Sets the 5th bit of A
int CPU::SET_5_A()
{
	// set the 5th bit
	reg_AF.hi |= (1 << 5);

	reg_PC.dat += 1;
	debugPrint("SET 5, A\n");
	return 4;
}

// SET 6, B
// Sets the 6th bit of B
int CPU::SET_6_B()
{
	// set the 6th bit
	reg_BC.hi |= (1 << 6);

	reg_PC.dat += 1;
	debugPrint("SET 6, B\n");
	return 4;
}

// SET 6, C
// Sets the 6th bit of C
int CPU::SET_6_C()
{
	// set the 6th bit
	reg_BC.lo |= (1 << 6);

	reg_PC.dat += 1;
	debugPrint("SET 6, C\n");
	return 4;
}

// SET 6, D
// Sets the 6th bit of D
int CPU::SET_6_D()
{
	// set the 6th bit
	reg_DE.hi |= (1 << 6);

	reg_PC.dat += 1;
	debugPrint("SET 6, D\n");
	return 4;
}

// SET 6, E
// Sets the 6th bit of E
int CPU::SET_6_E()
{
	// set the 6th bit
	reg_DE.lo |= (1 << 6);

	reg_PC.dat += 1;
	debugPrint("SET 6, E\n");
	return 4;
}

// SET 6, H
// Sets the 6th bit of H
int CPU::SET_6_H()
{
	// set the 6th bit
	reg_HL.hi |= (1 << 6);

	reg_PC.dat += 1;
	debugPrint("SET 6, H\n");
	return 4;
}

// SET 6, L
// Sets the 6th bit of L
int CPU::SET_6_L()
{
	// set the 6th bit
	reg_HL.lo |= (1 << 6);

	reg_PC.dat += 1;
	debugPrint("SET 6, L\n");
	return 4;
}

// SET 6, (HL)
// Sets the 6th bit of the value at memory address pointed to by HL
int CPU::SET_6_HLp()
{
	// set the 6th bit
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] | (1 << 6));

	reg_PC.dat += 1;
	debugPrint("SET 6, (HL)\n");
	return 4;
}

// SET 6, A
// Sets the 6th bit of A
int CPU::SET_6_A()
{
	// set the 6th bit
	reg_AF.hi |= (1 << 6);

	reg_PC.dat += 1;
	debugPrint("SET 6, A\n");
	return 4;
}

// SET 7, B
// Sets the 7th bit of B
int CPU::SET_7_B()
{
	// set the 7th bit
	reg_BC.hi |= (1 << 7);

	reg_PC.dat += 1;
	debugPrint("SET 7, B\n");
	return 4;
}

// SET 7, C
// Sets the 7th bit of C
int CPU::SET_7_C()
{
	// set the 7th bit
	reg_BC.lo |= (1 << 7);

	reg_PC.dat += 1;
	debugPrint("SET 7, C\n");
	return 4;
}

// SET 7, D
// Sets the 7th bit of D
int CPU::SET_7_D()
{
	// set the 7th bit
	reg_DE.hi |= (1 << 7);

	reg_PC.dat += 1;
	debugPrint("SET 7, D\n");
	return 4;
}

// SET 7, E
// Sets the 7th bit of E
int CPU::SET_7_E()
{
	// set the 7th bit
	reg_DE.lo |= (1 << 7);

	reg_PC.dat += 1;
	debugPrint("SET 7, E\n");
	return 4;
}

// SET 7, H
// Sets the 7th bit of H
int CPU::SET_7_H()
{
	// set the 7th bit
	reg_HL.hi |= (1 << 7);

	reg_PC.dat += 1;
	debugPrint("SET 7, H\n");
	return 4;
}

// SET 7, L
// Sets the 7th bit of L
int CPU::SET_7_L()
{
	// set the 7th bit
	reg_HL.lo |= (1 << 7);

	reg_PC.dat += 1;
	debugPrint("SET 7, L\n");
	return 4;
}

// SET 7, (HL)
// Sets the 7th bit of the value at memory address pointed to by HL
int CPU::SET_7_HLp()
{
	// set the 7th bit
	mMap->writeMemory(reg_HL.dat, (*mMap)[reg_HL.dat] | (1 << 7));

	reg_PC.dat += 1;
	debugPrint("SET 7, (HL)\n");
	return 4;
}

// SET 7, A
// Sets the 7th bit of A
int CPU::SET_7_A()
{
	// set the 7th bit
	reg_AF.hi |= (1 << 7);

	reg_PC.dat += 1;
	debugPrint("SET 7, A\n");
	return 4;
}

void CPU::dumpState()
{
	fprintf(outfile, "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n", reg_AF.hi, reg_AF.lo, reg_BC.hi, reg_BC.lo, reg_DE.hi, reg_DE.lo, reg_HL.hi, reg_HL.lo, reg_SP.dat, reg_PC.dat, (*mMap)[reg_PC.dat], (*mMap)[reg_PC.dat + 1], (*mMap)[reg_PC.dat + 2], (*mMap)[reg_PC.dat + 3]);
}

// Checks for interrupts and services them if needed
// Behaviour source: https://gbdev.io/pandocs/Interrupts.html
int CPU::performInterrupt()
{
	// check if interrupts must be enabled
	// after execution of opcode after EI
	// look at CPU::EI() for more info
	if (IMEFlag == 1)
		IMEReg = true;

	// If EI just executed
	// increment the flag
	if (!IMEFlag)
		IMEFlag = 1;

	// If IME is disabled
	// don't perform interrupt
	if (!IMEReg)
	{
		if (isHalted && (mMap->getRegIE() & mMap->getRegIF()))
		{
			isHalted = false;
			reg_PC.dat += 1;
		}
		return 0;
	}

	// Loop through all interrupts
	// In the priority order listed above
	for (int i = 0; i < 5; i++)
	{
		// Check if interrupt is enabled (IE at 0xFFFF), requested (IF at 0xFF0F) and IME is enabled
		if (((mMap->getRegIF() >> i) & 1) && ((mMap->getRegIE() >> i) & 1))
		{
			// Disable IME
			IMEReg = false;

			// Clear the interrupt flag as we are servicing it
			mMap->writeMemory(0xFF0F, mMap->getRegIF() ^ (1 << i));

			// Push PC onto stack if not halted
			// if halted, push PC + 1
			// and resume CPU execution
			if (!isHalted)
			{
				mMap->writeMemory(--reg_SP.dat, (reg_PC.dat) >> 8);
				mMap->writeMemory(--reg_SP.dat, (reg_PC.dat) & 0xFF);
			}
			else
			{
				mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) >> 8);
				mMap->writeMemory(--reg_SP.dat, (reg_PC.dat + 1) & 0xFF);
				isHalted = false;
			}

			// Jump to interrupt address
			// given in the interrupts array in cpu.h
			reg_PC.dat = interrupts[i];

			// Return 20 cycles
			return 20;
		}
	}

	// No interrupt to service
	// Return 0 cycles
	return 0;
}

// Updates the DIV and TIMA timers
// Calls for interrupts if necessary
// Behaviour source: https://gbdev.io/pandocs/Timer_and_Divider_Registers.html
void CPU::updateTimers(int cycles)
{
	// Update the reg_DIV register
	// Every 256 cycles
	timer_counter.div += cycles;

	if (timer_counter.div >= 0xFF)
	{
		timer_counter.div -= 0xFF;
		mMap->updateDividerRegister();
	}

	// check if timer is enabled
	if (mMap->getRegTAC() & 0x04)
	{
		// get the frequency from reg_TAC
		int freq = timer_counter.time_modes[mMap->getRegTAC() & 0x03];

		// check if TIMA has been overwritten by code
		if ((timer_counter.tima / freq) != mMap->getRegTIMA())
			timer_counter.tima = (mMap->getRegTIMA()) * freq;

		// increment the counter
		timer_counter.tima += cycles;

		// reset reg_TIMA by value of reg_TMA if overflow and call interrupt
		if (timer_counter.tima > (0xFF * freq))
		{
			timer_counter.tima = (mMap->getRegTMA() * freq);
			mMap->setRegIF(TIMER);
		}

		// Write reg_TIMA value by calculting from our counter
		mMap->setRegTIMA(timer_counter.tima / freq);
	}
}