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
	mMap->writeMemory(reg_BC.dat, reg_AF.hi);

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

	if (!(reg_BC.hi & 0x0F))
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
    if (!(reg_BC.lo & 0x0F))
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

// STOP
// Stops the CPU until an interrupt occurs
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
	if (!(reg_DE.hi & 0x0F))
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

	// set half carry flag if there was a borrow from bit 4
	// Only way to have a borrow is if you are left with 0x0F
	// TODO: Find a better way which doesn't involve comparing entire byte
	if ((reg_DE.hi & 0x0F) == 0x0F)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

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
	// Set zero flag to 0
	reg_AF.lo &= ~FLAG_ZERO_z;

	// Set subtract flag to 0
	reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// Set Half Carry flag to 1 if bit 3 is 1
	// Example: 0000 1000 will become 0001 0000
	if ((reg_AF.hi & 0x80) == 0x80)
	{
		reg_AF.lo |= FLAG_CARRY_c;
	}
	else
	{
		reg_AF.lo &= ~FLAG_CARRY_c;
	}

	// Shift A left by 1
	reg_AF.hi = (reg_AF.hi << 1) | (reg_AF.hi >> 7);

	reg_PC.dat += 1;
	printf("RLA\n");
	return 4;
}

// JR i8
// Add a signed 8 bit immediate value to the program counter
int CPU::JR_i8()
{
	// TODO: Check if this is correct
	reg_PC.dat += (Byte)(*mMap)[reg_PC.dat + 1];
	printf("JR i8\n");
	return 12;
}

// ADD HL, DE
// Add DE to HL
int CPU::ADD_HL_DE()
{
	// Set subtract flag to 0
	reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// Set Half Carry flag to 1 if bit 11 is 1
	// Example: 0000 1000 0000 0000 will become 0001 0000 0000 0000
	if ((reg_HL.dat & 0x0800) == 0x0800)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	// Set Carry flag to 1 if bit 15 is 1
	// Example: 1000 0000 0000 0000 will become 0000 0000 0000 0001
	if ((reg_HL.dat & 0x8000) == 0x8000)
	{
		reg_AF.lo |= FLAG_CARRY_c;
	}
	else
	{
		reg_AF.lo &= ~FLAG_CARRY_c;
	}

	// Add HL and DE
	reg_HL.dat += reg_DE.dat;

	reg_PC.dat += 1;
	printf("ADD HL, DE");
	return 8;
}

// LD A, (DE)
// Loads the contents of the memory address pointed to by DE into A
int CPU::LD_A_DE()
{
	reg_AF.hi = (*mMap)[reg_DE.dat];
	reg_PC.dat += 1;
	printf("LD A, (DE)\n");
	return 8;
}

// DEC DE
// Decrement DE
int CPU::DEC_DE()
{
	reg_DE.dat -= 1;
	reg_PC.dat += 1;
	printf("DEC DE\n");
	return 8;
}

// INC E
// Increment E
int CPU::INC_E()
{
	reg_DE.lo += 1;

	// Set zero flag to 1 if E is 0
	if (reg_DE.lo == 0)
	{
		reg_AF.lo |= FLAG_ZERO_z;
	}
	else
	{
		reg_AF.lo &= ~FLAG_ZERO_z;
	}

	// Set subtract flag to 0
	reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// There will only be 0000 if there was a half carry
	if (!(reg_DE.lo & 0x0F))
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	reg_PC.dat += 1;
	printf("INC E\n");
	return 4;
}

// DEC E
// Decrement E
int CPU::DEC_E()
{
	reg_DE.lo -= 1;

	// Set zero flag to 1 if E is 0
	if (reg_DE.lo == 0)
	{
		reg_AF.lo |= FLAG_ZERO_z;
	}
	else
	{
		reg_AF.lo &= ~FLAG_ZERO_z;
	}

	// Set subtract flag to 1
	reg_AF.lo |= FLAG_SUBTRACT_n;

	// Set half carry flag to 1 if there was a borrow from bit 4
	// Only way to have a borrow is if you are left with 0x0F
	// TODO: Find a better way which doesn't involve comparing entire byte
	if ((reg_DE.lo & 0x0F) == 0x0F)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	reg_PC.dat += 1;
	printf("DEC E\n");
	return 4;
}

// LD E, u8
// Loads an 8 bit immediate value into E
int CPU::LD_E_u8()
{
	reg_DE.lo = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	printf("LD E, u8\n");
	return 8;
}

// RRA
// Rotate A right through carry flag
int CPU::RRA()
{
	// Set zero flag to 0
	reg_AF.lo &= ~FLAG_ZERO_z;

	// Set subtract flag to 0
	reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// Set Half Carry flag to 0
	reg_AF.lo &= ~FLAG_HALF_CARRY_h;

	// Set Carry flag to 1 if bit 0 is 1
	// Example: 1000 0001 will become 0100 0000
	if (reg_AF.hi & 0x01)
	{
		reg_AF.lo |= FLAG_CARRY_c;
	}
	else
	{
		reg_AF.lo &= ~FLAG_CARRY_c;
	}

	// Shift A right by 1
	reg_AF.hi = (reg_AF.hi >> 1) | (reg_AF.hi << 7);

	reg_PC.dat += 1;
	printf("RRA\n");
	return 4;
}

// JR NZ, i8
// Add a signed 8 bit immediate value to the program counter if zero flag is 0
int CPU::JR_NZ_r8()
{
	if(!(reg_AF.lo & FLAG_ZERO_z))
	{
		reg_PC.dat += (Byte)(*mMap)[reg_PC.dat + 1];
	}

	//TODO: What's with branch and without branch?
	return 12;
}

// LD HL, u16
// Loads a 16 bit immediate value into HL
int CPU::LD_HL_u16()
{
	reg_HL.dat = ((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2];
    reg_PC.dat += 3;
    printf("LD HL, u16\n");
    return 12;
}

// LD (HL+), A
// Loads the contents of A into the memory address pointed to by HL and increments HL
int CPU::LD_HLp_A()
{
	mMap->writeMemory((*mMap)[reg_HL.dat], reg_AF.hi);
	reg_HL.dat += 1;
	reg_PC.dat += 1;
	printf("LD (HL+), A\n");
	return 8;
}

// INC HL
// Increment HL
int CPU::INC_HL()
{
	reg_HL.dat += 1;
	reg_PC.dat += 1;
	printf("INC HL\n");
	return 8;
}

// INC H
// Increment H
int CPU::INC_H()
{
	reg_HL.hi += 1;

	// Set zero flag to 1 if H is 0
	if (reg_HL.hi == 0)
	{
		reg_AF.lo |= FLAG_ZERO_z;
	}
	else
	{
		reg_AF.lo &= ~FLAG_ZERO_z;
	}

	// Set subtract flag to 0
	reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// There will only be 0000 if there was a half carry
	if (!(reg_HL.hi & 0x0F))
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	reg_PC.dat += 1;
	printf("INC H\n");
	return 4;
}

// DEC H
// Decrement H
int CPU::DEC_H()
{
	reg_HL.hi -= 1;

	// Set zero flag to 1 if H is 0
	if (reg_HL.hi == 0)
	{
		reg_AF.lo |= FLAG_ZERO_z;
	}
	else
	{
		reg_AF.lo &= ~FLAG_ZERO_z;
	}

	// Set subtract flag to 1
	reg_AF.lo |= FLAG_SUBTRACT_n;

	// Set half carry flag to 1 if there was a borrow from bit 4
	// Only way to have a borrow is if you are left with 0x0F
	if((reg_HL.hi & 0x0F) == 0x0F)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	reg_PC.dat += 1;
	return 4;
}

// LD H, u8
// Loads an 8 bit immediate value into H
int CPU::LD_H_u8()
{
	reg_HL.hi = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	printf("LD H, u8\n");
	return 8;
}

// DAA
// Decimal adjust register A
int CPU::DAA()
{
	return 0;
}

// JR Z, i8
// Add a signed 8 bit immediate value to the program counter if zero flag is 1
int CPU::JR_Z_r8()
{
	if (reg_AF.lo & FLAG_ZERO_z)
	{
		reg_PC.dat += (Byte)(*mMap)[reg_PC.dat + 1];
	}
}

// ADD HL, HL
// Add HL to HL
int CPU::ADD_HL_HL()
{
	// Set subtract flag to 0
	reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// Set half carry flag to 1 if there was a carry from bit 11
	// Example: 0000 1000 0000 0000 + 0000 1000 0000 0000 = 0001 0000 0000 0000
	if (reg_HL.dat & 0x0800)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	// Set carry flag to 1 if there was a carry from bit 15
	// Example: 1000 0000 0000 0000 + 1000 0000 0000 0000 = 0000 0000 0000 0000
	if (reg_HL.dat & 0x8000)
	{
		reg_AF.lo |= FLAG_CARRY_c;
	}
	else
	{
		reg_AF.lo &= ~FLAG_CARRY_c;
	}

	reg_HL.dat += reg_HL.dat;
	reg_PC.dat += 1;
	printf("ADD HL, HL\n");
	return 8;
}

// LD A, (HL+)
// Loads the contents of the memory address pointed to by HL into A and increments HL
int CPU::LD_A_HLp()
{
	reg_AF.hi = (*mMap)[reg_HL.dat];
	reg_HL.dat += 1;
	reg_PC.dat += 1;
	printf("LD A, (HL+)\n");
	return 8;
}

// DEC HL
// Decrement HL
int CPU::DEC_HL()
{
	reg_HL.dat -= 1;
	reg_PC.dat += 1;
	printf("DEC HL\n");
	return 8;
}

// INC L
// Increment L
int CPU::INC_L()
{
	reg_HL.lo += 1;

	// Set zero flag to 1 if L is 0
	if (reg_HL.lo == 0)
	{
		reg_AF.lo |= FLAG_ZERO_z;
	}
	else
	{
		reg_AF.lo &= ~FLAG_ZERO_z;
	}

	// Set subtract flag to 0
	reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// There will only be 0000 if there was a half carry
	if (!(reg_HL.lo & 0x0F))
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	reg_PC.dat += 1;
	printf("INC L\n");
	return 4;
}

// DEC L
// Decrement L
int CPU::DEC_L()
{
	reg_HL.lo -= 1;

	// Set zero flag to 1 if L is 0
	if (reg_HL.lo == 0)
	{
		reg_AF.lo |= FLAG_ZERO_z;
	}
	else
	{
		reg_AF.lo &= ~FLAG_ZERO_z;
	}

	// Set subtract flag to 1
	reg_AF.lo |= FLAG_SUBTRACT_n;

	// Set half carry flag to 1 if there was a borrow from bit 4
	// Only way to have a borrow is if you are left with 0x0F
	if ((reg_HL.lo & 0x0F) == 0x0F)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	reg_PC.dat += 1;
	return 4;
}

// LD L, u8
// Loads an 8 bit immediate value into L
int CPU::LD_L_u8()
{
	reg_HL.lo = (*mMap)[reg_PC.dat + 1];
	reg_PC.dat += 2;
	printf("LD L, u8\n");
	return 8;
}

// CPL
// Complement A
int CPU::CPL()
{
	reg_AF.hi = ~reg_AF.hi;
	reg_AF.lo |= FLAG_SUBTRACT_n;
	reg_AF.lo |= FLAG_HALF_CARRY_h;
	reg_PC.dat += 1;
	printf("CPL\n");
	return 4;
}

// JR NC, i8
// Add a signed 8 bit immediate value to the program counter if carry flag is 0
int CPU::JR_NC_i8()
{
	if (!(reg_AF.lo & FLAG_CARRY_c))
	{
		reg_PC.dat += (Byte)(*mMap)[reg_PC.dat + 1];
	}

	//TODO: Check if this is correct
	return 12;
}

// LD SP, u16
// Loads a 16 bit immediate value into SP
int CPU::LD_SP_u16()
{
	reg_SP.dat = ((*mMap)[reg_PC.dat + 1] << 8) | (*mMap)[reg_PC.dat + 2];
    reg_PC.dat += 3;
    printf("LD SP, u16\n");
    return 12;
}

// LD (HL-), A
// Loads the contents of A into the memory address pointed to by HL and decrements HL
int CPU::LD_HLm_A()
{
	mMap->writeMemory((*mMap)[reg_HL.dat], reg_AF.hi);
	reg_HL.dat -= 1;
	reg_PC.dat += 1;
	printf("LD (HL-), A\n");
	return 8;
}

// INC SP
// Increment SP
int CPU::INC_SP()
{
	reg_SP.dat += 1;
	reg_PC.dat += 1;
	printf("INC SP\n");
	return 8;
}

// INC (HL)
// Increment the contents of the memory address pointed to by HL
int CPU::INC_HLp()
{
	Byte temp = (*mMap)[reg_HL.dat];
	temp += 1;

	// Set zero flag to 1 if the result is 0
	if (temp == 0)
	{
		reg_AF.lo |= FLAG_ZERO_z;
	}
	else
	{
		reg_AF.lo &= ~FLAG_ZERO_z;
	}

	// Set subtract flag to 0
	reg_AF.lo &= ~FLAG_SUBTRACT_n;

	// Set half carry flag to 1 if there was a carry from bit 3
	// Example: 0000 1111 + 0000 0001 = 0001 0000
	if (temp & 0x10)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	mMap->writeMemory((*mMap)[reg_HL.dat], temp);
	reg_PC.dat += 1;
	printf("INC (HL)\n");
	return 12;
}

// DEC (HL)
// Decrement the contents of the memory address pointed to by HL
int CPU::DEC_HLp()
{
	Byte temp = (*mMap)[reg_HL.dat];
	temp -= 1;

	// Set zero flag to 1 if the result is 0
	if (temp == 0)
	{
		reg_AF.lo |= FLAG_ZERO_z;
	}
	else
	{
		reg_AF.lo &= ~FLAG_ZERO_z;
	}

	// Set subtract flag to 1
	reg_AF.lo |= FLAG_SUBTRACT_n;

	// Set half carry flag to 1 if there was a borrow from bit 4
	// Only way to have a borrow is if you are left with 0x0F
	if ((temp & 0x0F) == 0x0F)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
	}
	else
	{
		reg_AF.lo &= ~FLAG_HALF_CARRY_h;
	}

	mMap->writeMemory((*mMap)[reg_HL.dat], temp);
	reg_PC.dat += 1;
	printf("DEC (HL)\n");
	return 12;
}
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