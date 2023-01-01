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

	// ISA
    // Pulled from https://izik1.github.io/gbops/index.html
	typedef int (CPU::*method_function)();
	method_function method_pointer[0x01] = {
		&CPU::NOP
		// &CPU::LD_BC_u16,
		// &CPU::LD_BC_A,
		// &CPU::INC_BC,
		// &CPU::INC_B,
		// &CPU::DEC_B,
		// &CPU::LD_B_u8,
		// &CPU::RLCA,
		// &CPU::LD_u16_SP,
		// &CPU::ADD_HL_BC,
		// &CPU::LD_A_BC,
		// &CPU::DEC_BC,
		// &CPU::INC_C,
		// &CPU::DEC_C,
		// &CPU::LD_C_u8,
		// &CPU::RRCA,
		// &CPU::STOP,
		// &CPU::LD_DE_u16,
		// &CPU::LD_DE_A,
		// &CPU::INC_DE,
		// &CPU::INC_D,
		// &CPU::DEC_D,
		// &CPU::LD_D_u8,
		// &CPU::RLA,
		// &CPU::JR_r8,
		// &CPU::ADD_HL_DE,
		// &CPU::LD_A_DE,
		// &CPU::DEC_DE,
		// &CPU::INC_E,
		// &CPU::DEC_E,
		// &CPU::LD_E_u8,
		// &CPU::RRA,
		// &CPU::JR_NZ_r8,
		// &CPU::LD_HL_u16,
		// &CPU::LD_HLp_A,
		// &CPU::INC_HL,
		// &CPU::INC_H,
		// &CPU::DEC_H,
		// &CPU::LD_H_u8,
		// &CPU::DAA,
		// &CPU::JR_Z_r8,
		// &CPU::ADD_HL_HL,
		// &CPU::LD_A_HLp,
		// &CPU::DEC_HL,
		// &CPU::INC_L,
		// &CPU::DEC_L,
		// &CPU::LD_L_u8,
		// &CPU::CPL,
		// &CPU::JR_NC_r8,
		// &CPU::LD_SP_u16,
		// &CPU::LD_HLm_A,
		// &CPU::INC_SP,
		// &CPU::INC_HLp,
		// &CPU::DEC_HLp,
		// &CPU::LD_HLp_u8,
		// &CPU::SCF,
		// &CPU::JR_C_r8,
		// &CPU::ADD_HL_SP,
		// &CPU::LD_A_HLm,
		// &CPU::DEC_SP,
		// &CPU::INC_A,
		// &CPU::DEC_A,
		// &CPU::LD_A_u8,
		// &CPU::CCF,
		// &CPU::LD_B_B,
		// &CPU::LD_B_C,
		// &CPU::LD_B_D,
		// &CPU::LD_B_E,
		// &CPU::LD_B_H,
		// &CPU::LD_B_L,
		// &CPU::LD_B_HLp,
		// &CPU::LD_B_A,
		// &CPU::LD_C_B,
		// &CPU::LD_C_C,
		// &CPU::LD_C_D,
		// &CPU::LD_C_E,
		// &CPU::LD_C_H,
		// &CPU::LD_C_L,
		// &CPU::LD_C_HLp,
		// &CPU::LD_C_A,
		// &CPU::LD_D_B,
		// &CPU::LD_D_C,
		// &CPU::LD_D_D,
		// &CPU::LD_D_E,
		// &CPU::LD_D_H,
		// &CPU::LD_D_L,
		// &CPU::LD_D_HLp,
		// &CPU::LD_D_A,
		// &CPU::LD_E_B,
		// &CPU::LD_E_C,
		// &CPU::LD_E_D,
		// &CPU::LD_E_E,
		// &CPU::LD_E_H,
		// &CPU::LD_E_L,
		// &CPU::LD_E_HLp,
		// &CPU::LD_E_A,
		// &CPU::LD_H_B,
		// &CPU::LD_H_C,
		// &CPU::LD_H_D,
		// &CPU::LD_H_E,
		// &CPU::LD_H_H,
		// &CPU::LD_H_L,
		// &CPU::LD_H_HLp,
		// &CPU::LD_H_A,
		// &CPU::LD_L_B,
		// &CPU::LD_L_C,
		// &CPU::LD_L_D,
		// &CPU::LD_L_E,
		// &CPU::LD_L_H,
		// &CPU::LD_L_L,
		// &CPU::LD_L_HLp,
		// &CPU::LD_L_A,
		// &CPU::LD_HLp_B,
		// &CPU::LD_HLp_C,
		// &CPU::LD_HLp_D,
		// &CPU::LD_HLp_E,
		// &CPU::LD_HLp_H,
		// &CPU::LD_HLp_L,
		// &CPU::HALT,
		// &CPU::LD_HLp_A,
		// &CPU::LD_A_B,
		// &CPU::LD_A_C,
		// &CPU::LD_A_D,
		// &CPU::LD_A_E,
		// &CPU::LD_A_H,
		// &CPU::LD_A_L,
		// &CPU::LD_A_HLp,
		// &CPU::LD_A_A,
		// &CPU::ADD_A_B,
		// &CPU::ADD_A_C,
		// &CPU::ADD_A_D,
		// &CPU::ADD_A_E,
		// &CPU::ADD_A_H,
		// &CPU::ADD_A_L,
		// &CPU::ADD_A_HLp,
		// &CPU::ADD_A_A,
		// &CPU::ADC_A_B,
		// &CPU::ADC_A_C,
		// &CPU::ADC_A_D,
		// &CPU::ADC_A_E,
		// &CPU::ADC_A_H,
		// &CPU::ADC_A_L,
		// &CPU::ADC_A_HLp,
		// &CPU::ADC_A_A,
		// &CPU::SUB_B,
		// &CPU::SUB_C,
		// &CPU::SUB_D,
		// &CPU::SUB_E,
		// &CPU::SUB_H,
		// &CPU::SUB_L,
		// &CPU::SUB_HLp,
		// &CPU::SUB_A,
		// &CPU::SBC_A_B,
		// &CPU::SBC_A_C,
		// &CPU::SBC_A_D,
		// &CPU::SBC_A_E,
		// &CPU::SBC_A_H,
		// &CPU::SBC_A_L,
		// &CPU::SBC_A_HLp,
		// &CPU::SBC_A_A,
		// &CPU::AND_B,
		// &CPU::AND_C,
		// &CPU::AND_D,
		// &CPU::AND_E,
		// &CPU::AND_H,
		// &CPU::AND_L,
		// &CPU::AND_HLp,
		// &CPU::AND_A,
		// &CPU::XOR_B,
		// &CPU::XOR_C,
		// &CPU::XOR_D,
		// &CPU::XOR_E,
		// &CPU::XOR_H,
		// &CPU::XOR_L,
		// &CPU::XOR_HLp,
		// &CPU::XOR_A,
		// &CPU::OR_B,
		// &CPU::OR_C,
		// &CPU::OR_D,
		// &CPU::OR_E,
		// &CPU::OR_H,
		// &CPU::OR_L,
		// &CPU::OR_HLp,
		// &CPU::OR_A,
		// &CPU::CP_B,
		// &CPU::CP_C,
		// &CPU::CP_D,
		// &CPU::CP_E,
		// &CPU::CP_H,
		// &CPU::CP_L,
		// &CPU::CP_HLp,
		// &CPU::CP_A,
		// &CPU::RET_NZ,
		// &CPU::POP_BC,
		// &CPU::JP_NZ_u16,
		// &CPU::JP_u16,
		// &CPU::CALL_NZ_u16,
		// &CPU::PUSH_BC,
		// &CPU::ADD_A_u8,
		// &CPU::RST_00H,
		// &CPU::RET_Z,
		// &CPU::RET,
		// &CPU::JP_Z_u16,
		// &CPU::PREFIX_CB,
		// &CPU::CALL_Z_u16,
		// &CPU::CALL_u16,
		// &CPU::ADC_A_u8,
		// &CPU::RST_08H,
		// &CPU::RET_NC,
		// &CPU::POP_DE,
		// &CPU::JP_NC_u16,
		// &CPU::UNKNOWN,
		// &CPU::NC_u16,
		// &CPU::PUSH_DE,
		// &CPU::SUB_u8,
		// &CPU::RST_10H,
		// &CPU::RET_C,
		// &CPU::RETI,
		// &CPU::JP_C_u16,
		// &CPU::UNKNOWN,
		// &CPU::CALL_C_u16,
		// &CPU::UNKNOWN,
		// &CPU::SBC_A_u8,
		// &CPU::RST_18H,
		// &CPU::LDH_a8_A,
		// &CPU::POP_HL,
		// &CPU::LDH_C_A,
		// &CPU::UNKNOWN,
		// &CPU::UNKNOWN,
		// &CPU::PUSH_HL,
		// &CPU::AND_u8,
		// &CPU::RST_20H,
		// &CPU::ADD_SP_i8,
		// &CPU::JP_HL,
		// &CPU::LD_a16_A,
		// &CPU::UNKNOWN,
		// &CPU::UNKNOWN,
		// &CPU::UNKNOWN,
		// &CPU::XOR_u8,
		// &CPU::RST_28H,
		// &CPU::LDH_A_a8,
		// &CPU::POP_AF,
		// &CPU::LDH_A_C,
		// &CPU::DI,
		// &CPU::UNKNOWN,
		// &CPU::PUSH_AF,
		// &CPU::OR_u8,
		// &CPU::RST_30H,
		// &CPU::LD_HL_SP_i8,
		// &CPU::LD_SP_HL,
		// &CPU::LD_A_a16,
		// &CPU::EI,
		// &CPU::UNKNOWN,
		// &CPU::UNKNOWN,
		// &CPU::CP_u8,
		// &CPU::RST_38H
	};

    // NOP
    int NOP();

public:
	const int clockSpeed = 4194304; // 4.194304 MHz CPU
	const int clockSpeedPerFrame = 70224; // 4194304 / 59.73fps

	CPU();
	int executeNextInstruction(int opcode);
};