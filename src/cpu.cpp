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

int CPU::INC_BC()
{
	reg_BC.dat += 1;
    reg_PC.dat += 1;
    printf("INC BC\n");
    return 8;
}
int CPU::INC_B() { return 0; }
int CPU::DEC_B()
{
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

	if (reg_BC.hi == 0xFF)
	{
		reg_AF.lo |= FLAG_HALF_CARRY_h;
    }
    else
    {
        reg_AF.lo &= ~FLAG_HALF_CARRY_h;
    }

    reg_PC.dat += 1;
    printf("DEC B\n");
    return 4;
}
int CPU::LD_B_u8() { return 0; }
int CPU::RLCA() { return 0; }
int CPU::LD_u16_SP() { return 0; }
int CPU::ADD_HL_BC() { return 0; }
int CPU::LD_A_BC() { return 0; }
int CPU::DEC_BC() { return 0; }
int CPU::INC_C() { return 0; }
int CPU::DEC_C() { return 0; }
int CPU::LD_C_u8() { return 0; }
int CPU::RRCA() { return 0; }
int CPU::STOP() { return 0; }
int CPU::LD_DE_u16() { return 0; }
int CPU::LD_DE_A() { return 0; }
int CPU::INC_DE() { return 0; }
int CPU::INC_D() { return 0; }
int CPU::DEC_D() { return 0; }
int CPU::LD_D_u8() { return 0; }
int CPU::RLA() { return 0; }
int CPU::JR_r8() { return 0; }
int CPU::ADD_HL_DE() { return 0; }
int CPU::LD_A_DE() { return 0; }
int CPU::DEC_DE() { return 0; }
int CPU::INC_E() { return 0; }
int CPU::DEC_E() { return 0; }
int CPU::LD_E_u8() { return 0; }
int CPU::RRA() { return 0; }
int CPU::JR_NZ_r8() { return 0; }
int CPU::LD_HL_u16() { return 0; }
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
int CPU::LD_SP_u16() { return 0; }
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
int CPU::LD_B_B() { return 0; }
int CPU::LD_B_C() { return 0; }
int CPU::LD_B_D() { return 0; }
int CPU::LD_B_E() { return 0; }
int CPU::LD_B_H() { return 0; }
int CPU::LD_B_L() { return 0; }
int CPU::LD_B_HLp() { return 0; }
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
int CPU::UNKNOWN() { return 0; }
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
int CPU::LDH_a8_A() { return 0; }
int CPU::POP_HL() { return 0; }
int CPU::LDH_C_A() { return 0; }
int CPU::PUSH_HL() { return 0; }
int CPU::AND_u8() { return 0; }
int CPU::RST_20H() { return 0; }
int CPU::ADD_SP_i8() { return 0; }
int CPU::JP_HL() { return 0; }
int CPU::LD_a16_A() { return 0; }
int CPU::XOR_u8() { return 0; }
int CPU::RST_28H() { return 0; }
int CPU::LDH_A_a8() { return 0; }
int CPU::POP_AF() { return 0; }
int CPU::LDH_A_C() { return 0; }
int CPU::DI() { return 0; }
int CPU::PUSH_AF() { return 0; }
int CPU::OR_u8() { return 0; }
int CPU::RST_30H() { return 0; }
int CPU::LD_HL_SP_i8() { return 0; }
int CPU::LD_SP_HL() { return 0; }
int CPU::LD_A_a16() { return 0; }
int CPU::EI() { return 0; }
int CPU::CP_u8() { return 0; }
int CPU::RST_38H() { return 0; }

int CPU::executeNextInstruction()
{
	// Get the opcode
	Byte opcode = (*mMap)[reg_PC.dat];
	return (this->*method_pointer[opcode])();
}