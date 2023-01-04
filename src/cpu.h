#pragma once
#include "types.h"
#include "mmap.h"

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

	// Low PowerMode Bool
	bool isLowPower;

	// Flags
	// Pulled from https://gbdev.io/pandocs/CPU_Registers_and_Flags.html
	// Naming convention is: FLAG_<name>_<bit>
	// Bit 0-3 are not used
	enum Flags
	{
		FLAG_CARRY_c = 0x10,
		FLAG_HALF_CARRY_h = 0x20,
		FLAG_SUBTRACT_n = 0x40,
		FLAG_ZERO_z = 0x80
	};

	// Memory Map
	MemoryMap* mMap;

	// ISA
	// Pulled from https://izik1.github.io/gbops/index.html
	typedef int (CPU::*method_function)();
	method_function method_pointer[0x100] = {
		&CPU::NOP,
		&CPU::LD_BC_u16,
		&CPU::LD_BC_A,
		&CPU::INC_BC,
		&CPU::INC_B,
		&CPU::DEC_B,
		&CPU::LD_B_u8,
		&CPU::RLCA,
		&CPU::LD_u16_SP,
		&CPU::ADD_HL_BC,
		&CPU::LD_A_BC,
		&CPU::DEC_BC,
		&CPU::INC_C,
		&CPU::DEC_C,
		&CPU::LD_C_u8,
		&CPU::RRCA,
		&CPU::STOP,
		&CPU::LD_DE_u16,
		&CPU::LD_DE_A,
		&CPU::INC_DE,
		&CPU::INC_D,
		&CPU::DEC_D,
		&CPU::LD_D_u8,
		&CPU::RLA,
		&CPU::JR_i8,
		&CPU::ADD_HL_DE,
		&CPU::LD_A_DE,
		&CPU::DEC_DE,
		&CPU::INC_E,
		&CPU::DEC_E,
		&CPU::LD_E_u8,
		&CPU::RRA,
		&CPU::JR_NZ_r8,
		&CPU::LD_HL_u16,
		&CPU::LD_HLp_A,
		&CPU::INC_HL,
		&CPU::INC_H,
		&CPU::DEC_H,
		&CPU::LD_H_u8,
		&CPU::DAA,
		&CPU::JR_Z_r8,
		&CPU::ADD_HL_HL,
		&CPU::LD_A_HLp,
		&CPU::DEC_HL,
		&CPU::INC_L,
		&CPU::DEC_L,
		&CPU::LD_L_u8,
		&CPU::CPL,
		&CPU::JR_NC_i8,
		&CPU::LD_SP_u16,
		&CPU::LD_HLm_A,
		&CPU::INC_SP,
		&CPU::INC_HLp,
		&CPU::DEC_HLp,
		&CPU::LD_HLp_u8,
		&CPU::SCF,
		&CPU::JR_C_r8,
		&CPU::ADD_HL_SP,
		&CPU::LD_A_HLm,
		&CPU::DEC_SP,
		&CPU::INC_A,
		&CPU::DEC_A,
		&CPU::LD_A_u8,
		&CPU::CCF,
		&CPU::LD_B_B,
		&CPU::LD_B_C,
		&CPU::LD_B_D,
		&CPU::LD_B_E,
		&CPU::LD_B_H,
		&CPU::LD_B_L,
		&CPU::LD_B_HLp,
		&CPU::LD_B_A,
		&CPU::LD_C_B,
		&CPU::LD_C_C,
		&CPU::LD_C_D,
		&CPU::LD_C_E,
		&CPU::LD_C_H,
		&CPU::LD_C_L,
		&CPU::LD_C_HLp,
		&CPU::LD_C_A,
		&CPU::LD_D_B,
		&CPU::LD_D_C,
		&CPU::LD_D_D,
		&CPU::LD_D_E,
		&CPU::LD_D_H,
		&CPU::LD_D_L,
		&CPU::LD_D_HLp,
		&CPU::LD_D_A,
		&CPU::LD_E_B,
		&CPU::LD_E_C,
		&CPU::LD_E_D,
		&CPU::LD_E_E,
		&CPU::LD_E_H,
		&CPU::LD_E_L,
		&CPU::LD_E_HLp,
		&CPU::LD_E_A,
		&CPU::LD_H_B,
		&CPU::LD_H_C,
		&CPU::LD_H_D,
		&CPU::LD_H_E,
		&CPU::LD_H_H,
		&CPU::LD_H_L,
		&CPU::LD_H_HLp,
		&CPU::LD_H_A,
		&CPU::LD_L_B,
		&CPU::LD_L_C,
		&CPU::LD_L_D,
		&CPU::LD_L_E,
		&CPU::LD_L_H,
		&CPU::LD_L_L,
		&CPU::LD_L_HLp,
		&CPU::LD_L_A,
		&CPU::LD_HLp_B,
		&CPU::LD_HLp_C,
		&CPU::LD_HLp_D,
		&CPU::LD_HLp_E,
		&CPU::LD_HLp_H,
		&CPU::LD_HLp_L,
		&CPU::HALT,
		&CPU::LD_HLA,
		&CPU::LD_A_B,
		&CPU::LD_A_C,
		&CPU::LD_A_D,
		&CPU::LD_A_E,
		&CPU::LD_A_H,
		&CPU::LD_A_L,
		&CPU::LD_A_HL,
		&CPU::LD_A_A,
		&CPU::ADD_A_B,
		&CPU::ADD_A_C,
		&CPU::ADD_A_D,
		&CPU::ADD_A_E,
		&CPU::ADD_A_H,
		&CPU::ADD_A_L,
		&CPU::ADD_A_HLp,
		&CPU::ADD_A_A,
		&CPU::ADC_A_B,
		&CPU::ADC_A_C,
		&CPU::ADC_A_D,
		&CPU::ADC_A_E,
		&CPU::ADC_A_H,
		&CPU::ADC_A_L,
		&CPU::ADC_A_HLp,
		&CPU::ADC_A_A,
		&CPU::SUB_B,
		&CPU::SUB_C,
		&CPU::SUB_D,
		&CPU::SUB_E,
		&CPU::SUB_H,
		&CPU::SUB_L,
		&CPU::SUB_HLp,
		&CPU::SUB_A,
		&CPU::SBC_A_B,
		&CPU::SBC_A_C,
		&CPU::SBC_A_D,
		&CPU::SBC_A_E,
		&CPU::SBC_A_H,
		&CPU::SBC_A_L,
		&CPU::SBC_A_HLp,
		&CPU::SBC_A_A,
		&CPU::AND_B,
		&CPU::AND_C,
		&CPU::AND_D,
		&CPU::AND_E,
		&CPU::AND_H,
		&CPU::AND_L,
		&CPU::AND_HLp,
		&CPU::AND_A,
		&CPU::XOR_B,
		&CPU::XOR_C,
		&CPU::XOR_D,
		&CPU::XOR_E,
		&CPU::XOR_H,
		&CPU::XOR_L,
		&CPU::XOR_HLp,
		&CPU::XOR_A,
		&CPU::OR_B,
		&CPU::OR_C,
		&CPU::OR_D,
		&CPU::OR_E,
		&CPU::OR_H,
		&CPU::OR_L,
		&CPU::OR_HLp,
		&CPU::OR_A,
		&CPU::CP_B,
		&CPU::CP_C,
		&CPU::CP_D,
		&CPU::CP_E,
		&CPU::CP_H,
		&CPU::CP_L,
		&CPU::CP_HLp,
		&CPU::CP_A,
		&CPU::RET_NZ,
		&CPU::POP_BC,
		&CPU::JP_NZ_u16,
		&CPU::JP_u16,
		&CPU::CALL_NZ_u16,
		&CPU::PUSH_BC,
		&CPU::ADD_A_u8,
		&CPU::RST_00H,
		&CPU::RET_Z,
		&CPU::RET,
		&CPU::JP_Z_u16,
		&CPU::PREFIX_CB,
		&CPU::CALL_Z_u16,
		&CPU::CALL_u16,
		&CPU::ADC_A_u8,
		&CPU::RST_08H,
		&CPU::RET_NC,
		&CPU::POP_DE,
		&CPU::JP_NC_u16,
		&CPU::UNKNOWN,
		&CPU::NC_u16,
		&CPU::PUSH_DE,
		&CPU::SUB_u8,
		&CPU::RST_10H,
		&CPU::RET_C,
		&CPU::RETI,
		&CPU::JP_C_u16,
		&CPU::UNKNOWN,
		&CPU::CALL_C_u16,
		&CPU::UNKNOWN,
		&CPU::SBC_A_u8,
		&CPU::RST_18H,
		&CPU::LDH_a8_A,
		&CPU::POP_HL,
		&CPU::LDH_C_A,
		&CPU::UNKNOWN,
		&CPU::UNKNOWN,
		&CPU::PUSH_HL,
		&CPU::AND_u8,
		&CPU::RST_20H,
		&CPU::ADD_SP_i8,
		&CPU::JP_HL,
		&CPU::LD_a16_A,
		&CPU::UNKNOWN,
		&CPU::UNKNOWN,
		&CPU::UNKNOWN,
		&CPU::XOR_u8,
		&CPU::RST_28H,
		&CPU::LDH_A_a8,
		&CPU::POP_AF,
		&CPU::LDH_A_C,
		&CPU::DI,
		&CPU::UNKNOWN,
		&CPU::PUSH_AF,
		&CPU::OR_u8,
		&CPU::RST_30H,
		&CPU::LD_HL_SP_i8,
		&CPU::LD_SP_HL,
		&CPU::LD_A_a16,
		&CPU::EI,
		&CPU::UNKNOWN,
		&CPU::UNKNOWN,
		&CPU::CP_u8,
		&CPU::RST_38H
	};

	int NOP();
	int LD_BC_u16();
	int LD_BC_A();
	int INC_BC();
	int INC_B();
	int DEC_B();
	int LD_B_u8();
	int RLCA();
	int LD_u16_SP();
	int ADD_HL_BC();
	int LD_A_BC();
	int DEC_BC();
	int INC_C();
	int DEC_C();
	int LD_C_u8();
	int RRCA();
	int STOP();
	int LD_DE_u16();
	int LD_DE_A();
	int INC_DE();
	int INC_D();
	int DEC_D();
	int LD_D_u8();
	int RLA();
	int JR_i8();
	int ADD_HL_DE();
	int LD_A_DE();
	int DEC_DE();
	int INC_E();
	int DEC_E();
	int LD_E_u8();
	int RRA();
	int JR_NZ_r8();
	int LD_HL_u16();
	int LD_HLp_A();
	int INC_HL();
	int INC_H();
	int DEC_H();
	int LD_H_u8();
	int DAA();
	int JR_Z_r8();
	int ADD_HL_HL();
	int LD_A_HLp();
	int DEC_HL();
	int INC_L();
	int DEC_L();
	int LD_L_u8();
	int CPL();
	int JR_NC_i8();
	int LD_SP_u16();
	int LD_HLm_A();
	int INC_SP();
	int INC_HLp();
	int DEC_HLp();
	int LD_HLp_u8();
	int SCF();
	int JR_C_r8();
	int ADD_HL_SP();
	int LD_A_HLm();
	int DEC_SP();
	int INC_A();
	int DEC_A();
	int LD_A_u8();
	int CCF();
	int LD_B_B();
	int LD_B_C();
	int LD_B_D();
	int LD_B_E();
	int LD_B_H();
	int LD_B_L();
	int LD_B_HLp();
	int LD_B_A();
	int LD_C_B();
	int LD_C_C();
	int LD_C_D();
	int LD_C_E();
	int LD_C_H();
	int LD_C_L();
	int LD_C_HLp();
	int LD_C_A();
	int LD_D_B();
	int LD_D_C();
	int LD_D_D();
	int LD_D_E();
	int LD_D_H();
	int LD_D_L();
	int LD_D_HLp();
	int LD_D_A();
	int LD_E_B();
	int LD_E_C();
	int LD_E_D();
	int LD_E_E();
	int LD_E_H();
	int LD_E_L();
	int LD_E_HLp();
	int LD_E_A();
	int LD_H_B();
	int LD_H_C();
	int LD_H_D();
	int LD_H_E();
	int LD_H_H();
	int LD_H_L();
	int LD_H_HLp();
	int LD_H_A();
	int LD_L_B();
	int LD_L_C();
	int LD_L_D();
	int LD_L_E();
	int LD_L_H();
	int LD_L_L();
	int LD_L_HLp();
	int LD_L_A();
	int LD_HLp_B();
	int LD_HLp_C();
	int LD_HLp_D();
	int LD_HLp_E();
	int LD_HLp_H();
	int LD_HLp_L();
	int HALT();
	int LD_HLA();
	int LD_A_B();
	int LD_A_C();
	int LD_A_D();
	int LD_A_E();
	int LD_A_H();
	int LD_A_L();
	int LD_A_HL();
	int LD_A_A();
	int ADD_A_B();
	int ADD_A_C();
	int ADD_A_D();
	int ADD_A_E();
	int ADD_A_H();
	int ADD_A_L();
	int ADD_A_HLp();
	int ADD_A_A();
	int ADC_A_B();
	int ADC_A_C();
	int ADC_A_D();
	int ADC_A_E();
	int ADC_A_H();
	int ADC_A_L();
	int ADC_A_HLp();
	int ADC_A_A();
	int SUB_B();
	int SUB_C();
	int SUB_D();
	int SUB_E();
	int SUB_H();
	int SUB_L();
	int SUB_HLp();
	int SUB_A();
	int SBC_A_B();
	int SBC_A_C();
	int SBC_A_D();
	int SBC_A_E();
	int SBC_A_H();
	int SBC_A_L();
	int SBC_A_HLp();
	int SBC_A_A();
	int AND_B();
	int AND_C();
	int AND_D();
	int AND_E();
	int AND_H();
	int AND_L();
	int AND_HLp();
	int AND_A();
	int XOR_B();
	int XOR_C();
	int XOR_D();
	int XOR_E();
	int XOR_H();
	int XOR_L();
	int XOR_HLp();
	int XOR_A();
	int OR_B();
	int OR_C();
	int OR_D();
	int OR_E();
	int OR_H();
	int OR_L();
	int OR_HLp();
	int OR_A();
	int CP_B();
	int CP_C();
	int CP_D();
	int CP_E();
	int CP_H();
	int CP_L();
	int CP_HLp();
	int CP_A();
	int RET_NZ();
	int POP_BC();
	int JP_NZ_u16();
	int JP_u16();
	int CALL_NZ_u16();
	int PUSH_BC();
	int ADD_A_u8();
	int RST_00H();
	int RET_Z();
	int RET();
	int JP_Z_u16();
	int PREFIX_CB();
	int CALL_Z_u16();
	int CALL_u16();
	int ADC_A_u8();
	int RST_08H();
	int RET_NC();
	int POP_DE();
	int JP_NC_u16();
	int UNKNOWN();
	int NC_u16();
	int PUSH_DE();
	int SUB_u8();
	int RST_10H();
	int RET_C();
	int RETI();
	int JP_C_u16();
	int CALL_C_u16();
	int SBC_A_u8();
	int RST_18H();
	int LDH_a8_A();
	int POP_HL();
	int LDH_C_A();
	int PUSH_HL();
	int AND_u8();
	int RST_20H();
	int ADD_SP_i8();
	int JP_HL();
	int LD_a16_A();
	int XOR_u8();
	int RST_28H();
	int LDH_A_a8();
	int POP_AF();
	int LDH_A_C();
	int DI();
	int PUSH_AF();
	int OR_u8();
	int RST_30H();
	int LD_HL_SP_i8();
	int LD_SP_HL();
	int LD_A_a16();
	int EI();
	int CP_u8();
	int RST_38H();

public:
	const int clockSpeed = 4194304; // 4.194304 MHz CPU
	const int clockSpeedPerFrame = 70224; // 4194304 / 59.73fps

	CPU();

	// set the memory map
	void setMemory(MemoryMap* memory) { mMap = memory; }

	void set_reg_A(Byte value) { reg_AF.hi = value; }

	void set_reg_BC(Word value) { reg_BC.dat = value; }

	int executeNextInstruction();
};