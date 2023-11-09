#pragma once
#include "common/types.h"
#include "mmap.h"
#include "graphics.h"

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

	// halt bool
	// as isLowPower is set by STOP too
	bool isHalted;

	// IME Flag to enable interrupts on next opcode
	// The EI opcode sets the IME flag
	// After execution of opcode after EI
	// so we need a bool to check if we need to set
	// IME flag after the opcode in the update loop
	// If IMEFlag is -1, we disable interrupts;
	// If it is 0, we wait for next opcode to execute
	// If it is 1, we enable interrupts
	int IMEFlag;

	// IME Register to enable or disable interrupts
	bool IMEReg;

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

	// Interrupts
	// 0x0040 - V-Blank
	// 0x0048 - LCD STAT
	// 0x0050 - Timer
	// 0x0058 - Serial
	// 0x0060 - Joypad
	// PC must jump to these addresses to service the interrupt
	Word interrupts[5] = { 0x0040, 0x0048, 0x0050, 0x0058, 0x0060 };

	enum interrupt_name
	{
		V_BLANK = 0x01,
		LCD_STAT = 0x02,
		TIMER = 0x04,
		SERIAL = 0x08,
		JOYPAD = 0x10
	};

	// Timer counter structs
	// Pulled from https://gbdev.io/pandocs/Timer_and_Divider_Registers.html
	// div increments mMap->reg_DIV at 16384Hz
	// tima increments mMap->reg_TIMA at the frequency specified by mMap->reg_TAC
	// time_modes is the frequency of the timer corresponding to
	// first two bits of mMap->reg_TAC
	struct
	{
		int div;
		int tima;
		int time_modes[4] = { 1024, 16, 64, 256 };
	} timer_counter;

	// Memory Map
	MemoryMap* mMap;

	PPU* ppu;

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
		&CPU::JR_NZ_i8,
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
		&CPU::SUB_A_B,
		&CPU::SUB_A_C,
		&CPU::SUB_A_D,
		&CPU::SUB_A_E,
		&CPU::SUB_A_H,
		&CPU::SUB_A_L,
		&CPU::SUB_A_HLp,
		&CPU::SUB_A_A,
		&CPU::SBC_A_B,
		&CPU::SBC_A_C,
		&CPU::SBC_A_D,
		&CPU::SBC_A_E,
		&CPU::SBC_A_H,
		&CPU::SBC_A_L,
		&CPU::SBC_A_HLp,
		&CPU::SBC_A_A,
		&CPU::AND_A_B,
		&CPU::AND_A_C,
		&CPU::AND_A_D,
		&CPU::AND_A_E,
		&CPU::AND_A_H,
		&CPU::AND_A_L,
		&CPU::AND_A_HLp,
		&CPU::AND_A_A,
		&CPU::XOR_A_B,
		&CPU::XOR_A_C,
		&CPU::XOR_A_D,
		&CPU::XOR_A_E,
		&CPU::XOR_A_H,
		&CPU::XOR_A_L,
		&CPU::XOR_A_HLp,
		&CPU::XOR_A_A,
		&CPU::OR_A_B,
		&CPU::OR_A_C,
		&CPU::OR_A_D,
		&CPU::OR_A_E,
		&CPU::OR_A_H,
		&CPU::OR_A_L,
		&CPU::OR_A_HLp,
		&CPU::OR_A_A,
		&CPU::CP_A_B,
		&CPU::CP_A_C,
		&CPU::CP_A_D,
		&CPU::CP_A_E,
		&CPU::CP_A_H,
		&CPU::CP_A_L,
		&CPU::CP_A_HLp,
		&CPU::CP_A_A,
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
		&CPU::AND_A_u8,
		&CPU::RST_20H,
		&CPU::ADD_SP_i8,
		&CPU::JP_HL,
		&CPU::LD_u16_A,
		&CPU::UNKNOWN,
		&CPU::UNKNOWN,
		&CPU::UNKNOWN,
		&CPU::XOR_A_u8,
		&CPU::RST_28H,
		&CPU::LDH_A_a8,
		&CPU::POP_AF,
		&CPU::LDH_A_C,
		&CPU::DI,
		&CPU::UNKNOWN,
		&CPU::PUSH_AF,
		&CPU::OR_A_u8,
		&CPU::RST_30H,
		&CPU::LD_HL_SP_i8,
		&CPU::LD_SP_HL,
		&CPU::LD_A_u16,
		&CPU::EI,
		&CPU::UNKNOWN,
		&CPU::UNKNOWN,
		&CPU::CP_u8,
		&CPU::RST_38H
	};

	method_function prefixed_method_pointer[0x100] = {
		&CPU::RLC_B,
		&CPU::RLC_C,
		&CPU::RLC_D,
		&CPU::RLC_E,
		&CPU::RLC_H,
		&CPU::RLC_L,
		&CPU::RLC_HLp,
		&CPU::RLC_A,
		&CPU::RRC_B,
		&CPU::RRC_C,
		&CPU::RRC_D,
		&CPU::RRC_E,
		&CPU::RRC_H,
		&CPU::RRC_L,
		&CPU::RRC_HLp,
		&CPU::RRC_A,
		&CPU::RL_B,
		&CPU::RL_C,
		&CPU::RL_D,
		&CPU::RL_E,
		&CPU::RL_H,
		&CPU::RL_L,
		&CPU::RL_HLp,
		&CPU::RL_A,
		&CPU::RR_B,
		&CPU::RR_C,
		&CPU::RR_D,
		&CPU::RR_E,
		&CPU::RR_H,
		&CPU::RR_L,
		&CPU::RR_HLp,
		&CPU::RR_A,
		&CPU::SLA_B,
		&CPU::SLA_C,
		&CPU::SLA_D,
		&CPU::SLA_E,
		&CPU::SLA_H,
		&CPU::SLA_L,
		&CPU::SLA_HLp,
		&CPU::SLA_A,
		&CPU::SRA_B,
		&CPU::SRA_C,
		&CPU::SRA_D,
		&CPU::SRA_E,
		&CPU::SRA_H,
		&CPU::SRA_L,
		&CPU::SRA_HLp,
		&CPU::SRA_A,
		&CPU::SWAP_B,
		&CPU::SWAP_C,
		&CPU::SWAP_D,
		&CPU::SWAP_E,
		&CPU::SWAP_H,
		&CPU::SWAP_L,
		&CPU::SWAP_HLp,
		&CPU::SWAP_A,
		&CPU::SRL_B,
		&CPU::SRL_C,
		&CPU::SRL_D,
		&CPU::SRL_E,
		&CPU::SRL_H,
		&CPU::SRL_L,
		&CPU::SRL_HLp,
		&CPU::SRL_A,
		&CPU::BIT_0_B,
		&CPU::BIT_0_C,
		&CPU::BIT_0_D,
		&CPU::BIT_0_E,
		&CPU::BIT_0_H,
		&CPU::BIT_0_L,
		&CPU::BIT_0_HLp,
		&CPU::BIT_0_A,
		&CPU::BIT_1_B,
		&CPU::BIT_1_C,
		&CPU::BIT_1_D,
		&CPU::BIT_1_E,
		&CPU::BIT_1_H,
		&CPU::BIT_1_L,
		&CPU::BIT_1_HLp,
		&CPU::BIT_1_A,
		&CPU::BIT_2_B,
		&CPU::BIT_2_C,
		&CPU::BIT_2_D,
		&CPU::BIT_2_E,
		&CPU::BIT_2_H,
		&CPU::BIT_2_L,
		&CPU::BIT_2_HLp,
		&CPU::BIT_2_A,
		&CPU::BIT_3_B,
		&CPU::BIT_3_C,
		&CPU::BIT_3_D,
		&CPU::BIT_3_E,
		&CPU::BIT_3_H,
		&CPU::BIT_3_L,
		&CPU::BIT_3_HLp,
		&CPU::BIT_3_A,
		&CPU::BIT_4_B,
		&CPU::BIT_4_C,
		&CPU::BIT_4_D,
		&CPU::BIT_4_E,
		&CPU::BIT_4_H,
		&CPU::BIT_4_L,
		&CPU::BIT_4_HLp,
		&CPU::BIT_4_A,
		&CPU::BIT_5_B,
		&CPU::BIT_5_C,
		&CPU::BIT_5_D,
		&CPU::BIT_5_E,
		&CPU::BIT_5_H,
		&CPU::BIT_5_L,
		&CPU::BIT_5_HLp,
		&CPU::BIT_5_A,
		&CPU::BIT_6_B,
		&CPU::BIT_6_C,
		&CPU::BIT_6_D,
		&CPU::BIT_6_E,
		&CPU::BIT_6_H,
		&CPU::BIT_6_L,
		&CPU::BIT_6_HLp,
		&CPU::BIT_6_A,
		&CPU::BIT_7_B,
		&CPU::BIT_7_C,
		&CPU::BIT_7_D,
		&CPU::BIT_7_E,
		&CPU::BIT_7_H,
		&CPU::BIT_7_L,
		&CPU::BIT_7_HLp,
		&CPU::BIT_7_A,
		&CPU::RES_0_B,
		&CPU::RES_0_C,
		&CPU::RES_0_D,
		&CPU::RES_0_E,
		&CPU::RES_0_H,
		&CPU::RES_0_L,
		&CPU::RES_0_HLp,
		&CPU::RES_0_A,
		&CPU::RES_1_B,
		&CPU::RES_1_C,
		&CPU::RES_1_D,
		&CPU::RES_1_E,
		&CPU::RES_1_H,
		&CPU::RES_1_L,
		&CPU::RES_1_HLp,
		&CPU::RES_1_A,
		&CPU::RES_2_B,
		&CPU::RES_2_C,
		&CPU::RES_2_D,
		&CPU::RES_2_E,
		&CPU::RES_2_H,
		&CPU::RES_2_L,
		&CPU::RES_2_HLp,
		&CPU::RES_2_A,
		&CPU::RES_3_B,
		&CPU::RES_3_C,
		&CPU::RES_3_D,
		&CPU::RES_3_E,
		&CPU::RES_3_H,
		&CPU::RES_3_L,
		&CPU::RES_3_HLp,
		&CPU::RES_3_A,
		&CPU::RES_4_B,
		&CPU::RES_4_C,
		&CPU::RES_4_D,
		&CPU::RES_4_E,
		&CPU::RES_4_H,
		&CPU::RES_4_L,
		&CPU::RES_4_HLp,
		&CPU::RES_4_A,
		&CPU::RES_5_B,
		&CPU::RES_5_C,
		&CPU::RES_5_D,
		&CPU::RES_5_E,
		&CPU::RES_5_H,
		&CPU::RES_5_L,
		&CPU::RES_5_HLp,
		&CPU::RES_5_A,
		&CPU::RES_6_B,
		&CPU::RES_6_C,
		&CPU::RES_6_D,
		&CPU::RES_6_E,
		&CPU::RES_6_H,
		&CPU::RES_6_L,
		&CPU::RES_6_HLp,
		&CPU::RES_6_A,
		&CPU::RES_7_B,
		&CPU::RES_7_C,
		&CPU::RES_7_D,
		&CPU::RES_7_E,
		&CPU::RES_7_H,
		&CPU::RES_7_L,
		&CPU::RES_7_HLp,
		&CPU::RES_7_A,
		&CPU::SET_0_B,
		&CPU::SET_0_C,
		&CPU::SET_0_D,
		&CPU::SET_0_E,
		&CPU::SET_0_H,
		&CPU::SET_0_L,
		&CPU::SET_0_HLp,
		&CPU::SET_0_A,
		&CPU::SET_1_B,
		&CPU::SET_1_C,
		&CPU::SET_1_D,
		&CPU::SET_1_E,
		&CPU::SET_1_H,
		&CPU::SET_1_L,
		&CPU::SET_1_HLp,
		&CPU::SET_1_A,
		&CPU::SET_2_B,
		&CPU::SET_2_C,
		&CPU::SET_2_D,
		&CPU::SET_2_E,
		&CPU::SET_2_H,
		&CPU::SET_2_L,
		&CPU::SET_2_HLp,
		&CPU::SET_2_A,
		&CPU::SET_3_B,
		&CPU::SET_3_C,
		&CPU::SET_3_D,
		&CPU::SET_3_E,
		&CPU::SET_3_H,
		&CPU::SET_3_L,
		&CPU::SET_3_HLp,
		&CPU::SET_3_A,
		&CPU::SET_4_B,
		&CPU::SET_4_C,
		&CPU::SET_4_D,
		&CPU::SET_4_E,
		&CPU::SET_4_H,
		&CPU::SET_4_L,
		&CPU::SET_4_HLp,
		&CPU::SET_4_A,
		&CPU::SET_5_B,
		&CPU::SET_5_C,
		&CPU::SET_5_D,
		&CPU::SET_5_E,
		&CPU::SET_5_H,
		&CPU::SET_5_L,
		&CPU::SET_5_HLp,
		&CPU::SET_5_A,
		&CPU::SET_6_B,
		&CPU::SET_6_C,
		&CPU::SET_6_D,
		&CPU::SET_6_E,
		&CPU::SET_6_H,
		&CPU::SET_6_L,
		&CPU::SET_6_HLp,
		&CPU::SET_6_A,
		&CPU::SET_7_B,
		&CPU::SET_7_C,
		&CPU::SET_7_D,
		&CPU::SET_7_E,
		&CPU::SET_7_H,
		&CPU::SET_7_L,
		&CPU::SET_7_HLp,
		&CPU::SET_7_A
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
	int JR_NZ_i8();
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
	int SUB_A_B();
	int SUB_A_C();
	int SUB_A_D();
	int SUB_A_E();
	int SUB_A_H();
	int SUB_A_L();
	int SUB_A_HLp();
	int SUB_A_A();
	int SBC_A_B();
	int SBC_A_C();
	int SBC_A_D();
	int SBC_A_E();
	int SBC_A_H();
	int SBC_A_L();
	int SBC_A_HLp();
	int SBC_A_A();
	int AND_A_B();
	int AND_A_C();
	int AND_A_D();
	int AND_A_E();
	int AND_A_H();
	int AND_A_L();
	int AND_A_HLp();
	int AND_A_A();
	int XOR_A_B();
	int XOR_A_C();
	int XOR_A_D();
	int XOR_A_E();
	int XOR_A_H();
	int XOR_A_L();
	int XOR_A_HLp();
	int XOR_A_A();
	int OR_A_B();
	int OR_A_C();
	int OR_A_D();
	int OR_A_E();
	int OR_A_H();
	int OR_A_L();
	int OR_A_HLp();
	int OR_A_A();
	int CP_A_B();
	int CP_A_C();
	int CP_A_D();
	int CP_A_E();
	int CP_A_H();
	int CP_A_L();
	int CP_A_HLp();
	int CP_A_A();
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
	int AND_A_u8();
	int RST_20H();
	int ADD_SP_i8();
	int JP_HL();
	int LD_u16_A();
	int XOR_A_u8();
	int RST_28H();
	int LDH_A_a8();
	int POP_AF();
	int LDH_A_C();
	int DI();
	int PUSH_AF();
	int OR_A_u8();
	int RST_30H();
	int LD_HL_SP_i8();
	int LD_SP_HL();
	int LD_A_u16();
	int EI();
	int CP_u8();
	int RST_38H();

	int RLC_B();
	int RLC_C();
	int RLC_D();
	int RLC_E();
	int RLC_H();
	int RLC_L();
	int RLC_HLp();
	int RLC_A();
	int RRC_B();
	int RRC_C();
	int RRC_D();
	int RRC_E();
	int RRC_H();
	int RRC_L();
	int RRC_HLp();
	int RRC_A();
	int RL_B();
	int RL_C();
	int RL_D();
	int RL_E();
	int RL_H();
	int RL_L();
	int RL_HLp();
	int RL_A();
	int RR_B();
	int RR_C();
	int RR_D();
	int RR_E();
	int RR_H();
	int RR_L();
	int RR_HLp();
	int RR_A();
	int SLA_B();
	int SLA_C();
	int SLA_D();
	int SLA_E();
	int SLA_H();
	int SLA_L();
	int SLA_HLp();
	int SLA_A();
	int SRA_B();
	int SRA_C();
	int SRA_D();
	int SRA_E();
	int SRA_H();
	int SRA_L();
	int SRA_HLp();
	int SRA_A();
	int SWAP_B();
	int SWAP_C();
	int SWAP_D();
	int SWAP_E();
	int SWAP_H();
	int SWAP_L();
	int SWAP_HLp();
	int SWAP_A();
	int SRL_B();
	int SRL_C();
	int SRL_D();
	int SRL_E();
	int SRL_H();
	int SRL_L();
	int SRL_HLp();
	int SRL_A();
	int BIT_0_B();
	int BIT_0_C();
	int BIT_0_D();
	int BIT_0_E();
	int BIT_0_H();
	int BIT_0_L();
	int BIT_0_HLp();
	int BIT_0_A();
	int BIT_1_B();
	int BIT_1_C();
	int BIT_1_D();
	int BIT_1_E();
	int BIT_1_H();
	int BIT_1_L();
	int BIT_1_HLp();
	int BIT_1_A();
	int BIT_2_B();
	int BIT_2_C();
	int BIT_2_D();
	int BIT_2_E();
	int BIT_2_H();
	int BIT_2_L();
	int BIT_2_HLp();
	int BIT_2_A();
	int BIT_3_B();
	int BIT_3_C();
	int BIT_3_D();
	int BIT_3_E();
	int BIT_3_H();
	int BIT_3_L();
	int BIT_3_HLp();
	int BIT_3_A();
	int BIT_4_B();
	int BIT_4_C();
	int BIT_4_D();
	int BIT_4_E();
	int BIT_4_H();
	int BIT_4_L();
	int BIT_4_HLp();
	int BIT_4_A();
	int BIT_5_B();
	int BIT_5_C();
	int BIT_5_D();
	int BIT_5_E();
	int BIT_5_H();
	int BIT_5_L();
	int BIT_5_HLp();
	int BIT_5_A();
	int BIT_6_B();
	int BIT_6_C();
	int BIT_6_D();
	int BIT_6_E();
	int BIT_6_H();
	int BIT_6_L();
	int BIT_6_HLp();
	int BIT_6_A();
	int BIT_7_B();
	int BIT_7_C();
	int BIT_7_D();
	int BIT_7_E();
	int BIT_7_H();
	int BIT_7_L();
	int BIT_7_HLp();
	int BIT_7_A();
	int RES_0_B();
	int RES_0_C();
	int RES_0_D();
	int RES_0_E();
	int RES_0_H();
	int RES_0_L();
	int RES_0_HLp();
	int RES_0_A();
	int RES_1_B();
	int RES_1_C();
	int RES_1_D();
	int RES_1_E();
	int RES_1_H();
	int RES_1_L();
	int RES_1_HLp();
	int RES_1_A();
	int RES_2_B();
	int RES_2_C();
	int RES_2_D();
	int RES_2_E();
	int RES_2_H();
	int RES_2_L();
	int RES_2_HLp();
	int RES_2_A();
	int RES_3_B();
	int RES_3_C();
	int RES_3_D();
	int RES_3_E();
	int RES_3_H();
	int RES_3_L();
	int RES_3_HLp();
	int RES_3_A();
	int RES_4_B();
	int RES_4_C();
	int RES_4_D();
	int RES_4_E();
	int RES_4_H();
	int RES_4_L();
	int RES_4_HLp();
	int RES_4_A();
	int RES_5_B();
	int RES_5_C();
	int RES_5_D();
	int RES_5_E();
	int RES_5_H();
	int RES_5_L();
	int RES_5_HLp();
	int RES_5_A();
	int RES_6_B();
	int RES_6_C();
	int RES_6_D();
	int RES_6_E();
	int RES_6_H();
	int RES_6_L();
	int RES_6_HLp();
	int RES_6_A();
	int RES_7_B();
	int RES_7_C();
	int RES_7_D();
	int RES_7_E();
	int RES_7_H();
	int RES_7_L();
	int RES_7_HLp();
	int RES_7_A();
	int SET_0_B();
	int SET_0_C();
	int SET_0_D();
	int SET_0_E();
	int SET_0_H();
	int SET_0_L();
	int SET_0_HLp();
	int SET_0_A();
	int SET_1_B();
	int SET_1_C();
	int SET_1_D();
	int SET_1_E();
	int SET_1_H();
	int SET_1_L();
	int SET_1_HLp();
	int SET_1_A();
	int SET_2_B();
	int SET_2_C();
	int SET_2_D();
	int SET_2_E();
	int SET_2_H();
	int SET_2_L();
	int SET_2_HLp();
	int SET_2_A();
	int SET_3_B();
	int SET_3_C();
	int SET_3_D();
	int SET_3_E();
	int SET_3_H();
	int SET_3_L();
	int SET_3_HLp();
	int SET_3_A();
	int SET_4_B();
	int SET_4_C();
	int SET_4_D();
	int SET_4_E();
	int SET_4_H();
	int SET_4_L();
	int SET_4_HLp();
	int SET_4_A();
	int SET_5_B();
	int SET_5_C();
	int SET_5_D();
	int SET_5_E();
	int SET_5_H();
	int SET_5_L();
	int SET_5_HLp();
	int SET_5_A();
	int SET_6_B();
	int SET_6_C();
	int SET_6_D();
	int SET_6_E();
	int SET_6_H();
	int SET_6_L();
	int SET_6_HLp();
	int SET_6_A();
	int SET_7_B();
	int SET_7_C();
	int SET_7_D();
	int SET_7_E();
	int SET_7_H();
	int SET_7_L();
	int SET_7_HLp();
	int SET_7_A();

public:
	const int clockSpeed = 4194304; // 4.194304 MHz CPU
	const int clockSpeedPerFrame = 70224; // 4194304 / 59.73fps

	CPU();

	// set the memory map
	void setMemory(MemoryMap* memory) { mMap = memory; }

	// set the PPU
	void setPPU(PPU* ppu_arg) { ppu = ppu_arg; }

	// set the Accumulator
	void set_reg_A(Byte value) { reg_AF.hi = value; }

	// set the BC register
	void set_reg_BC(Word value) { reg_BC.dat = value; }

	// get the Program Counter
	Word get_reg_PC() { return reg_PC.dat; }

	// get the HL register
	Word get_reg_HL() { return reg_HL.dat; }

	// execute an arbitrary instruction
	int executeInstruction(Byte opcode);

	// execute the next instruction
	int executeNextInstruction();

	// execute the next prefixed instruction
	int executePrefixedInstruction();

	// service interrupts
	int performInterrupt();

	// update the timers
	void updateTimers(int cycles);
};
