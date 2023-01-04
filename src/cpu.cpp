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

	// Set isLowPower to false
	isLowPower = false;
}

// NOP just adds 4 cycles
// Does nothing
int CPU::NOP()
{
	reg_PC.dat += 1;
	printf("NOP\n");
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
	printf("LD BC, u16\n");
	return 12;
}

// LD (BC), A
// Loads the contents of A into the memory address pointed to by BC
int CPU::LD_BC_A()
{
	// Write the contents of A into the memory address pointed to by BC
	mMap->writeMemory(reg_BC.dat, reg_AF.lo);

	// Increment the program counter
	reg_PC.dat += 1;

	printf("LD (BC), A\n");
	return 8;
}

// INC BC
// Increments the contents of BC
int CPU::INC_BC()
{
	reg_BC.dat += 1;
    reg_PC.dat += 1;
    printf("INC BC\n");
    return 8;
}

// INC B
// Increments the contents of B
int CPU::INC_B()
{
	reg_BC.hi += 1;
	if (reg_BC.hi == 0)
	{
		reg_AF.lo |= FLAG_ZERO_z;
    }
    else
    {
        reg_AF.lo &= ~FLAG_ZERO_z;
	}

	reg_AF.lo &= ~FLAG_SUBTRACT_n;

	if ((reg_BC.hi & 0x0F) == 0)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
    }
    else
    {
        reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}
    reg_PC.dat += 1;
    printf("INC B\n");
    return 4;
}

// DEC B
// Decrements the contents of B
// TODO: Dec B can be checked if B is 0001 0000
int CPU::DEC_B()
{
	bool flag = (reg_BC.hi & 0x10) == 0x10;
	reg_BC.hi -= 1;
	if (reg_BC.hi == 0)
	{
        reg_AF.lo |= FLAG_ZERO_z;
    }
    else
    {
        reg_AF.lo &= ~FLAG_ZERO_z;
    }

	reg_AF.lo |= FLAG_SUBTRACT_n;

	if ((reg_BC.hi & 0x10) == 0x00 && flag)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
    }
    else
    {
        reg_AF.lo &= ~FLAG_HALF_CARRY_h;
    }

    reg_PC.dat += 1;
    printf("DEC B\n" );
    return 4;
}

// LD B, u8
// Loads an 8 bit immediate value into B
int CPU::LD_B_u8()
{
	reg_BC.hi = (*mMap)[reg_PC.dat + 1];
    reg_PC.dat += 2;
    printf("LD B, u8\n");
    return 8;
}

// RLCA
// Rotate A left
int CPU::RLCA()
{
    reg_AF.lo &= ~FLAG_ZERO_z;
    reg_AF.lo &= ~FLAG_SUBTRACT_n;
    reg_AF.lo &= ~FLAG_HALF_CARRY_h;

	// store bit 7 in carry flag
    if (reg_AF.hi >> 7 == 1)
    {
        reg_AF.lo |= FLAG_CARRY_c;
    }
    else
    {
        reg_AF.lo &= ~FLAG_CARRY_c;
    }

	// Rotate A left by 1
    reg_AF.hi = (reg_AF.hi << 1) | (reg_AF.hi >> 7);

    reg_PC.dat += 1;
    printf("RLCA\n");
    return 4;
}

// LD (u16), SP
// Loads the contents of SP into the memory address pointed to by the next 2 bytes
int CPU::LD_u16_SP()
{
	// Load the next 2 bytes into a 16 bit value
    // Left shift the first byte by 8 bits
    // OR the second byte
    Word address = ((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2];

    // Write the contents of SP into the memory address pointed to by the next 2 bytes
    mMap->writeMemory(address, reg_SP.dat & 0xFF);
    mMap->writeMemory(address + 1, reg_SP.dat >> 8);

    // Increment the program counter
    reg_PC.dat += 3;

    printf("LD (u16), SP\n");
    return 20;
}

// ADD HL, BC
// Adds the contents of BC to HL
int CPU::ADD_HL_BC()
{
	// set subtract flag to 0
    reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// set carry flag if there is a carry from bit 11
    if ((reg_HL.dat + reg_BC.dat) >> 16 == 1)
    {
        reg_AF.lo |= FLAG_CARRY_c;
    }
    else
    {
        reg_AF.lo &= ~FLAG_CARRY_c;
    }

    reg_HL.dat += reg_BC.dat;

    reg_PC.dat += 1;
    printf("ADD HL, BC\n");
    return 8;
}

// LD A, (BC)
// Loads the contents of the memory address pointed to by BC into A
int CPU::LD_A_BC()
{
	reg_AF.lo = (*mMap)[reg_BC.dat];
    reg_PC.dat += 1;
    printf("LD A, (BC)\n");
    return 8;
}

// DEC BC
// Decrement BC
int CPU::DEC_BC()
{
	reg_BC.dat -= 1;
    reg_PC.dat += 1;
    printf("DEC BC\n");
    return 8;
}

// INC C
// Increment C
int CPU::INC_C()
{
    reg_BC.lo += 1;
    if (reg_BC.lo == 0)
    {
        reg_AF.lo |= FLAG_ZERO_z;
    }
    else
    {
        reg_AF.lo &= ~FLAG_ZERO_z;
    }

    reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// There will only be 0000 if there was a half carry
    if ((reg_BC.lo & 0x0F) == 0)
    {
        reg_AF.lo |= FLAG_HALF_CARRY_h;
    }
    else
    {
        reg_AF.lo &= ~FLAG_HALF_CARRY_h;
    }

    reg_PC.dat += 1;
    printf("INC C\n");
    return 4;
}

// DEC C
// Decrement C
int CPU::DEC_C()
{
	reg_BC.lo -= 1;

	// set zero flag if C is 0
    if (reg_BC.lo == 0)
    {
        reg_AF.lo |= FLAG_ZERO_z;
    }
    else
    {
        reg_AF.lo &= ~FLAG_ZERO_z;
    }

	// set subtract flag to 1
    reg_AF.lo |= FLAG_SUBTRACT_n;


    reg_PC.dat += 1;
    printf("DEC C\n");
    return 4;
}

// LD C, u8
// Loads an 8 bit immediate value into C
int CPU::LD_C_u8()
{
	reg_BC.lo = (*mMap)[reg_PC.dat + 1];
    reg_PC.dat += 2;
    printf("LD C, u8\n");
    return 8;
}

// RRCA
// Rotate A right
int CPU::RRCA()
{
	// Set zero flag to 0
    reg_AF.lo &= ~FLAG_ZERO_z;

	// Set subtract flag to 0
    reg_AF.lo &= ~FLAG_SUBTRACT_n;

    if ((reg_AF.hi & 1) == 1)
    {
        reg_AF.lo |= FLAG_CARRY_c;
    }
    else
    {
        reg_AF.lo &= ~FLAG_CARRY_c;
    }

    reg_AF.hi = (reg_AF.hi >> 1) | (reg_AF.hi << 7);

    reg_PC.dat += 1;
    printf("RRCA\n");
    return 4;
}
int CPU::STOP()
{
	isLowPower = true;
	reg_PC.dat += 2;
	printf("STOP\n");
    return 0;
}

// LD DE, u16
// Loads a 16 bit immediate value into DE
int CPU::LD_DE_u16()
{
	reg_DE.dat = ((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2];
    reg_PC.dat += 3;
    printf("LD DE, u16\n");
    return 12;
}

// LD (DE), A
// Loads the contents of A into the memory address pointed to by DE
int CPU::LD_DE_A()
{
	mMap->writeMemory((*mMap)[reg_DE.dat], reg_AF.hi);
	reg_PC.dat += 1;
	printf("LD (DE), A\n");
	return 8;
}

// INC DE
// Increment DE
int CPU::INC_DE()
{
	reg_DE.dat += 1;
    reg_PC.dat += 1;
    printf("INC DE\n");
    return 8;
}

// INC D
// Increment D
int CPU::INC_D()
{
	reg_DE.hi += 1;
	if (reg_DE.hi == 0)
	{
		reg_AF.lo |= FLAG_ZERO_z;
	}
	else
	{
		reg_AF.lo &= ~FLAG_ZERO_z;
	}

	reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// There will only be 0000 if there was a half carry
	if ((reg_DE.hi & 0x0F) == 0)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	reg_PC.dat += 1;
	printf("INC D\n");
	return 4;
}

// DEC D
// Decrement D
int CPU::DEC_D()
{
    reg_DE.hi -= 1;

	// set zero flag if D is 0
    if (reg_DE.hi == 0)
    {
        reg_AF.lo |= FLAG_ZERO_z;
    }
    else
    {
        reg_AF.lo &= ~FLAG_ZERO_z;
    }

	//set subtract flag to 1
    reg_AF.lo |= FLAG_SUBTRACT_n;

    reg_PC.dat += 1;
    printf("DEC D\n");
    return 4;
}

// LD D, u8
// Loads an 8 bit immediate value into D
int CPU::LD_D_u8()
{
	reg_DE.hi = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	printf("LD D, u8\n");
	return 8;
}

// RLA
// Rotate A left through carry flag
int CPU::RLA()
{
	return 0;
}
int CPU::JR_r8() { return 0; }
int CPU::ADD_HL_DE() { return 0; }
int CPU::LD_A_DE() { return 0; }
int CPU::DEC_DE() { return 0; }
int CPU::INC_E() { return 0; }
int CPU::DEC_E() { return 0; }
int CPU::LD_E_u8() { return 0; }
int CPU::RRA() { return 0; }
int CPU::JR_NZ_r8() { return 0; }

// LD HL, u16
// Loads a 16 bit immediate value into HL
int CPU::LD_HL_u16()
{
	reg_HL.dat = ((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2];
    reg_PC.dat += 3;
    printf("LD HL, u16\n");
    return 12;
}
int CPU::LD_HLp_A() { return 0; }
int CPU::INC_HL() { return 0; }
int CPU::INC_H() { return 0; }
int CPU::DEC_H() { return 0; }
int CPU::LD_H_u8() { return 0; }
int CPU::DAA() { return 0; }
int CPU::JR_Z_r8() { return 0; }
int CPU::ADD_HL_HL() { return 0; }
int CPU::LD_A_HLp() { return 0; }
int CPU::DEC_HL() { return 0; }
int CPU::INC_L() { return 0; }
int CPU::DEC_L() { return 0; }
int CPU::LD_L_u8() { return 0; }
int CPU::CPL() { return 0; }
int CPU::JR_NC_r8() { return 0; }
int CPU::LD_SP_u16()
{
	reg_SP.dat = ((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2];
    reg_PC.dat += 3;
    printf("LD SP, u16\n");
    return 12;
}
int CPU::LD_HLm_A() { return 0; }
int CPU::INC_SP() { return 0; }
int CPU::INC_HLp() { return 0; }
int CPU::DEC_HLp() { return 0; }
int CPU::LD_HLp_u8() { return 0; }
int CPU::SCF() { return 0; }
int CPU::JR_C_r8() { return 0; }
int CPU::ADD_HL_SP() { return 0; }
int CPU::LD_A_HLm() { return 0; }
int CPU::DEC_SP() { return 0; }
int CPU::INC_A() { return 0; }
int CPU::DEC_A() { return 0; }
int CPU::LD_A_u8() { return 0; }
int CPU::CCF() { return 0; }

// LD B, B
// Loads B into B
int CPU::LD_B_B()
{
	reg_BC.hi = reg_BC.hi;
    reg_PC.dat += 1;
    printf("LD B, B\n");
    return 4;
}
int CPU::LD_B_C() { return 0; }

// LD B, D
// Loads D into B
int CPU::LD_B_D()
{
	reg_BC.hi = reg_DE.hi;
    reg_PC.dat += 1;
    printf("LD B, D\n");
    return 4;
}
int CPU::LD_B_E() { return 0; }

// LD B, H
// Loads H into B
int CPU::LD_B_H()
{
	reg_BC.hi = reg_HL.hi;
    reg_PC.dat += 1;
    printf("LD B, H\n");
    return 4;
}
int CPU::LD_B_L() { return 0; }

// LD B, (HL)
// Loads the value at address HL into B
int CPU::LD_B_HLp()
{
	reg_BC.hi = (*mMap)[reg_HL.dat];
    reg_PC.dat += 1;
    printf("LD B, (HL)\n");
    return 8;
}
int CPU::LD_B_A() { return 0; }
int CPU::LD_C_B() { return 0; }
int CPU::LD_C_C() { return 0; }
int CPU::LD_C_D() { return 0; }
int CPU::LD_C_E() { return 0; }
int CPU::LD_C_H() { return 0; }
int CPU::LD_C_L() { return 0; }
int CPU::LD_C_HLp() { return 0; }
int CPU::LD_C_A() { return 0; }
int CPU::LD_D_B() { return 0; }
int CPU::LD_D_C() { return 0; }
int CPU::LD_D_D() { return 0; }
int CPU::LD_D_E() { return 0; }
int CPU::LD_D_H() { return 0; }
int CPU::LD_D_L() { return 0; }
int CPU::LD_D_HLp() { return 0; }
int CPU::LD_D_A() { return 0; }
int CPU::LD_E_B() { return 0; }
int CPU::LD_E_C() { return 0; }
int CPU::LD_E_D() { return 0; }
int CPU::LD_E_E() { return 0; }
int CPU::LD_E_H() { return 0; }
int CPU::LD_E_L() { return 0; }
int CPU::LD_E_HLp() { return 0; }
int CPU::LD_E_A() { return 0; }
int CPU::LD_H_B() { return 0; }
int CPU::LD_H_C() { return 0; }
int CPU::LD_H_D() { return 0; }
int CPU::LD_H_E() { return 0; }
int CPU::LD_H_H() { return 0; }
int CPU::LD_H_L() { return 0; }
int CPU::LD_H_HLp() { return 0; }
int CPU::LD_H_A() { return 0; }
int CPU::LD_L_B() { return 0; }
int CPU::LD_L_C() { return 0; }
int CPU::LD_L_D() { return 0; }
int CPU::LD_L_E() { return 0; }
int CPU::LD_L_H() { return 0; }
int CPU::LD_L_L() { return 0; }
int CPU::LD_L_HLp() { return 0; }
int CPU::LD_L_A() { return 0; }
int CPU::LD_HLp_B() { return 0; }
int CPU::LD_HLp_C() { return 0; }
int CPU::LD_HLp_D() { return 0; }
int CPU::LD_HLp_E() { return 0; }
int CPU::LD_HLp_H() { return 0; }
int CPU::LD_HLp_L() { return 0; }
int CPU::HALT() { return 0; }
int CPU::LD_HLA() { return 0; }
int CPU::LD_A_B() { return 0; }
int CPU::LD_A_C() { return 0; }
int CPU::LD_A_D() { return 0; }
int CPU::LD_A_E() { return 0; }
int CPU::LD_A_H() { return 0; }
int CPU::LD_A_L() { return 0; }
int CPU::LD_A_HL() { return 0; }
int CPU::LD_A_A() { return 0; }
int CPU::ADD_A_B() { return 0; }
int CPU::ADD_A_C() { return 0; }
int CPU::ADD_A_D() { return 0; }
int CPU::ADD_A_E() { return 0; }
int CPU::ADD_A_H() { return 0; }
int CPU::ADD_A_L() { return 0; }
int CPU::ADD_A_HLp() { return 0; }
int CPU::ADD_A_A() { return 0; }
int CPU::ADC_A_B() { return 0; }
int CPU::ADC_A_C() { return 0; }
int CPU::ADC_A_D() { return 0; }
int CPU::ADC_A_E() { return 0; }
int CPU::ADC_A_H() { return 0; }
int CPU::ADC_A_L() { return 0; }
int CPU::ADC_A_HLp() { return 0; }
int CPU::ADC_A_A() { return 0; }
int CPU::SUB_B() { return 0; }
int CPU::SUB_C() { return 0; }
int CPU::SUB_D() { return 0; }
int CPU::SUB_E() { return 0; }
int CPU::SUB_H() { return 0; }
int CPU::SUB_L() { return 0; }
int CPU::SUB_HLp() { return 0; }
int CPU::SUB_A() { return 0; }
int CPU::SBC_A_B() { return 0; }
int CPU::SBC_A_C() { return 0; }
int CPU::SBC_A_D() { return 0; }
int CPU::SBC_A_E() { return 0; }
int CPU::SBC_A_H() { return 0; }
int CPU::SBC_A_L() { return 0; }
int CPU::SBC_A_HLp() { return 0; }
int CPU::SBC_A_A() { return 0; }
int CPU::AND_B() { return 0; }
int CPU::AND_C() { return 0; }
int CPU::AND_D() { return 0; }
int CPU::AND_E() { return 0; }
int CPU::AND_H() { return 0; }
int CPU::AND_L() { return 0; }
int CPU::AND_HLp() { return 0; }
int CPU::AND_A() { return 0; }
int CPU::XOR_B() { return 0; }
int CPU::XOR_C() { return 0; }
int CPU::XOR_D() { return 0; }
int CPU::XOR_E() { return 0; }
int CPU::XOR_H() { return 0; }
int CPU::XOR_L() { return 0; }
int CPU::XOR_HLp() { return 0; }
int CPU::XOR_A() { return 0; }
int CPU::OR_B() { return 0; }
int CPU::OR_C() { return 0; }
int CPU::OR_D() { return 0; }
int CPU::OR_E() { return 0; }
int CPU::OR_H() { return 0; }
int CPU::OR_L() { return 0; }
int CPU::OR_HLp() { return 0; }
int CPU::OR_A() { return 0; }
int CPU::CP_B() { return 0; }
int CPU::CP_C() { return 0; }
int CPU::CP_D() { return 0; }
int CPU::CP_E() { return 0; }
int CPU::CP_H() { return 0; }
int CPU::CP_L() { return 0; }
int CPU::CP_HLp() { return 0; }
int CPU::CP_A() { return 0; }
int CPU::RET_NZ() { return 0; }
int CPU::POP_BC() { return 0; }
int CPU::JP_NZ_u16() { return 0; }
int CPU::JP_u16() { return 0; }
int CPU::CALL_NZ_u16() { return 0; }
int CPU::PUSH_BC() { return 0; }
int CPU::ADD_A_u8() { return 0; }
int CPU::RST_00H() { return 0; }
int CPU::RET_Z() { return 0; }
int CPU::RET() { return 0; }
int CPU::JP_Z_u16() { return 0; }
int CPU::PREFIX_CB() { return 0; }
int CPU::CALL_Z_u16() { return 0; }
int CPU::CALL_u16() { return 0; }
int CPU::ADC_A_u8() { return 0; }
int CPU::RST_08H() { return 0; }
int CPU::RET_NC() { return 0; }
int CPU::POP_DE() { return 0; }
int CPU::JP_NC_u16() { return 0; }

int CPU::UNKNOWN()
{
	const char *s = NULL;
	printf( "%c\n", s[0] );
	return 0;
}
int CPU::NC_u16() { return 0; }
int CPU::PUSH_DE() { return 0; }
int CPU::SUB_u8() { return 0; }
int CPU::RST_10H() { return 0; }
int CPU::RET_C() { return 0; }
int CPU::RETI() { return 0; }
int CPU::JP_C_u16() { return 0; }
int CPU::CALL_C_u16() { return 0; }
int CPU::SBC_A_u8() { return 0; }
int CPU::RST_18H() { return 0; }
// LDH (a8), A
// Load A into (0xFF00 + a8)
int CPU::LDH_a8_A()
{
    return 0;
}
int CPU::POP_HL() { return 0; }
int CPU::LDH_C_A() { return 0; }
int CPU::PUSH_HL() { return 0; }
int CPU::AND_u8() { return 0; }
int CPU::RST_20H() { return 0; }
int CPU::ADD_SP_i8() { return 0; }
int CPU::JP_HL() { return 0; }

// LD (u16), A
// Load A into (u16)
int CPU::LD_a16_A()
{
	// u16 is ((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2]
	// Writing the value of A into the (u16)
	mMap->debugWriteMemory(((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2],(*mMap)[reg_AF.hi]);
	reg_PC.dat += 3;
	printf("LD (u16), A\n");
	return 16;
}
int CPU::XOR_u8() { return 0; }
int CPU::RST_28H() { return 0; }
int CPU::LDH_A_a8() { return 0; }
int CPU::POP_AF() { return 0; }

// LDH A, (C)
// Load (0xFF00 + C) into A
int CPU::LDH_A_C()
{
	reg_AF.hi = (*mMap)[0xFF00 + reg_BC.lo];
	reg_PC.dat += 1;
    return 8;
}
int CPU::DI() { return 0; }
int CPU::PUSH_AF() { return 0; }
int CPU::OR_u8() { return 0; }
int CPU::RST_30H() { return 0; }
// LD HL, SP + i8
// Load SP + i8 into HL
int CPU::LD_HL_SP_i8()
{
    reg_HL.dat = reg_SP.dat + (Byte)(*mMap)[reg_PC.dat + 1];
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
int CPU::LD_A_a16()
{
	reg_AF.hi = (*mMap)[((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2]];
    reg_PC.dat += 3;
    printf("LD A, (HL)\n");
    return 16;
}
int CPU::EI() { return 0; }
int CPU::CP_u8() { return 0; }
int CPU::RST_38H() { return 0; }

int CPU::executeNextInstruction()
{
	// Get the opcode
	Byte opcode = (*mMap)[reg_PC.dat];
	return (this->*method_pointer[opcode])();
}