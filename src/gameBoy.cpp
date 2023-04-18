#include "types.h"
#include "cpu.h"
#include "gameBoy.h"

int GBE::s_Cycles;

GBE::GBE()
{
	// Initialize the CPU
	gbe_cpu = new CPU();

	// Initialize the MemoryMap
	gbe_mMap = new MemoryMap();

	// Initialize the Graphics
	gbe_graphics = new PPU();

	// Unify the CPU and MemoryMap
	gbe_cpu->setMemory(gbe_mMap);

	// Unify the CPU and PPU
	gbe_cpu->setPPU(gbe_graphics);

	// Unify the PPU and MmeoryMap
	gbe_graphics->setMemoryMap(gbe_mMap);

	gbe_graphics->init();

	// Open the Boot ROM
	if ((bootROM = fopen("../src/dmg_boot.gb", "rb")) == NULL)
		printf("boot rom file not opened");

	// Open the Game ROM
	if ((gameROM = fopen("../tests/Tetris.gb", "rb")) == NULL)
		printf("game rom file not opened");

	// Load the Boot ROM
	// Into the first 0x100 bytes
	fread(gbe_mMap->getRomBank0(), 1, 256, bootROM);

	// Load Game ROM in Bank 0
	// After offsetting for Boot ROM first
	fseek(gameROM, 0x100, SEEK_SET);

	fread(gbe_mMap->getRomBank0() + 0x100, 1, 16128, gameROM);
	fread(gbe_mMap->getRomBank1(), 1, 16384, gameROM);

	s_Cycles = 0;

	// STUB: Fooling the emulator
	// Into thinking the frame is ready
	// Helps us get out of the loop at
	// 0x0064 in the boot ROM
	// Needs to be removed once Timers
	// and PPU is implemented
	gbe_mMap->writeMemory(0xff44, 0x90);

	// Adding the Nintendo Logo to ROM
	// to pass the Boot check

	gbe_mMap->debugWriteMemory(0x104, 0xCE);
	gbe_mMap->debugWriteMemory(0x105, 0xED);
	gbe_mMap->debugWriteMemory(0x106, 0x66);
	gbe_mMap->debugWriteMemory(0x107, 0x66);
	gbe_mMap->debugWriteMemory(0x108, 0xCC);
	gbe_mMap->debugWriteMemory(0x109, 0x0D);
	gbe_mMap->debugWriteMemory(0x10A, 0x00);
	gbe_mMap->debugWriteMemory(0x10B, 0x0B);
	gbe_mMap->debugWriteMemory(0x10C, 0x03);
	gbe_mMap->debugWriteMemory(0x10D, 0x73);
	gbe_mMap->debugWriteMemory(0x10E, 0x00);
	gbe_mMap->debugWriteMemory(0x10F, 0x83);
	gbe_mMap->debugWriteMemory(0x110, 0x00);
	gbe_mMap->debugWriteMemory(0x111, 0x0C);
	gbe_mMap->debugWriteMemory(0x112, 0x00);
	gbe_mMap->debugWriteMemory(0x113, 0x0D);
	gbe_mMap->debugWriteMemory(0x114, 0x00);
	gbe_mMap->debugWriteMemory(0x115, 0x08);
	gbe_mMap->debugWriteMemory(0x116, 0x11);
	gbe_mMap->debugWriteMemory(0x117, 0x1F);
	gbe_mMap->debugWriteMemory(0x118, 0x88);
	gbe_mMap->debugWriteMemory(0x119, 0x89);
	gbe_mMap->debugWriteMemory(0x11A, 0x00);
	gbe_mMap->debugWriteMemory(0x11B, 0x0E);
	gbe_mMap->debugWriteMemory(0x11C, 0xDC);
	gbe_mMap->debugWriteMemory(0x11D, 0xCC);
	gbe_mMap->debugWriteMemory(0x11E, 0x6E);
	gbe_mMap->debugWriteMemory(0x11F, 0xE6);
	gbe_mMap->debugWriteMemory(0x120, 0xDD);
	gbe_mMap->debugWriteMemory(0x121, 0xDD);
	gbe_mMap->debugWriteMemory(0x122, 0xD9);
	gbe_mMap->debugWriteMemory(0x123, 0x99);
	gbe_mMap->debugWriteMemory(0x124, 0xBB);
	gbe_mMap->debugWriteMemory(0x125, 0xBB);
	gbe_mMap->debugWriteMemory(0x126, 0x67);
	gbe_mMap->debugWriteMemory(0x127, 0x63);
	gbe_mMap->debugWriteMemory(0x128, 0x6E);
	gbe_mMap->debugWriteMemory(0x129, 0x0E);
	gbe_mMap->debugWriteMemory(0x12A, 0xEC);
	gbe_mMap->debugWriteMemory(0x12B, 0xCC);
	gbe_mMap->debugWriteMemory(0x12C, 0xDD);
	gbe_mMap->debugWriteMemory(0x12D, 0xDC);
	gbe_mMap->debugWriteMemory(0x12E, 0x99);
	gbe_mMap->debugWriteMemory(0x12F, 0x9F);
	gbe_mMap->debugWriteMemory(0x130, 0xBB);
	gbe_mMap->debugWriteMemory(0x131, 0xB9);
	gbe_mMap->debugWriteMemory(0x132, 0x33);
	gbe_mMap->debugWriteMemory(0x133, 0x3E);

	// Using the Tetris header to pass the checksum test
	// Unnecessary if loading a valid rom

	/*gbe_mMap->debugWriteMemory(0x134, 0x54);
	gbe_mMap->debugWriteMemory(0x135, 0x45);
	gbe_mMap->debugWriteMemory(0x136, 0x54);
	gbe_mMap->debugWriteMemory(0x137, 0x52);
	gbe_mMap->debugWriteMemory(0x138, 0x49);
	gbe_mMap->debugWriteMemory(0x139, 0x53);
	gbe_mMap->debugWriteMemory(0x13A, 0x00);
	gbe_mMap->debugWriteMemory(0x13B, 0x00);
	gbe_mMap->debugWriteMemory(0x13C, 0x00);
	gbe_mMap->debugWriteMemory(0x13D, 0x00);
	gbe_mMap->debugWriteMemory(0x13E, 0x00);
	gbe_mMap->debugWriteMemory(0x13F, 0x00);
	gbe_mMap->debugWriteMemory(0x140, 0x00);
	gbe_mMap->debugWriteMemory(0x141, 0x00);
	gbe_mMap->debugWriteMemory(0x142, 0x00);
	gbe_mMap->debugWriteMemory(0x143, 0x00);
	gbe_mMap->debugWriteMemory(0x144, 0x00);
	gbe_mMap->debugWriteMemory(0x145, 0x00);
	gbe_mMap->debugWriteMemory(0x146, 0x00);
	gbe_mMap->debugWriteMemory(0x147, 0x00);
	gbe_mMap->debugWriteMemory(0x148, 0x00);
	gbe_mMap->debugWriteMemory(0x149, 0x00);
	gbe_mMap->debugWriteMemory(0x14A, 0x00);
	gbe_mMap->debugWriteMemory(0x14B, 0x01);
	gbe_mMap->debugWriteMemory(0x14C, 0x01);
	gbe_mMap->debugWriteMemory(0x14D, 0x0A);*/

	// I have commented my tests for now as the Nintendo logo now sits there.
	// We can test using blargg's test rom then.

	//// Current State: AF = 0x01B0, BC = 0x0013, DE = 0x00D8, HL = 0x014D, SP = 0xFFFE, PC = 0x0100

	//// NOP
	//// Does nothing
	//// Final State: No change
	// gbe_mMap->debugWriteMemory(0x0100, 0x00);

	//// LD BC, u16
	//// Loads a 16 bit immediate into the register BC
	//// Final State: BC = 0XE001
	// gbe_mMap->debugWriteMemory(0x0101, 0x01);
	// gbe_mMap->debugWriteMemory(0x0102, 0x01);
	// gbe_mMap->debugWriteMemory(0x0103, 0xE0);

	//// LD (BC), A
	//// Loads the value of the accumulator into the memory address pointed to by BC
	//// Final State: Value 0x01 at address 0xE001 (2nd byte of Work RAM)
	// gbe_mMap->debugWriteMemory(0x0104, 0x02);

	//// INC BC
	//// Increments the value of BC by 1
	//// Final State: BC = 0xE002
	// gbe_mMap->debugWriteMemory(0x0105, 0x03);

	//// INC B
	//// Increments the value of B by 1
	//// Final State: BC = 0xE102, Flag_N = 0, Flag_H = 0, Flag_Z = 0, AF = 0110
	// gbe_mMap->debugWriteMemory(0x0106, 0x04);

	//// Set BC = 0XFFFF to test INC B for Z flag
	// gbe_mMap->debugWriteMemory(0x0107, 0x01);
	// gbe_mMap->debugWriteMemory(0x0108, 0xFF);
	// gbe_mMap->debugWriteMemory(0x0109, 0xFF);

	//// INC B
	//// Increments the value of B by 1
	//// Final State: BC = 0x00FF, Flag_N = 0, Flag_H = 1, Flag_Z = 1, AF = 01B0
	// gbe_mMap->debugWriteMemory(0x010A, 0x04);

	//// Set BC = 0X0FFF to test INC B for H flag
	// gbe_mMap->debugWriteMemory(0x010B, 0x01);
	// gbe_mMap->debugWriteMemory(0x010C, 0xFF);
	// gbe_mMap->debugWriteMemory(0x010D, 0x0F);

	//// INC B
	//// Increments the value of B by 1
	//// Final State: BC = 0x10FF, Flag_N = 0, Flag_H = 1, Flag_Z = 0, AF = 0130
	// gbe_mMap->debugWriteMemory(0x010E, 0x04);

	//// DEC B
	//// Decrements the value of B by 1
	//// Final State: BC = 0x0FFF, Flag_N = 1, Flag_H = 1, Flag_Z = 0, AF = 0170
	// gbe_mMap->debugWriteMemory(0x010F, 0x05);

	//// Set BC = 0X01FF to test DEC B for Z flag
	// gbe_mMap->debugWriteMemory(0x0110, 0x01);
	// gbe_mMap->debugWriteMemory(0x0111, 0xFF);
	// gbe_mMap->debugWriteMemory(0x0112, 0x01);

	//// DEC B
	//// Decrements the value of B by 1
	//// Final State: BC = 0x00FF, Flag_N = 1, Flag_H = 0, Flag_Z = 1, AF = 01D0
	// gbe_mMap->debugWriteMemory(0x0113, 0x05);

	//// Set BC = 0X020F to test DEC B
	// gbe_mMap->debugWriteMemory(0x0114, 0x01);
	// gbe_mMap->debugWriteMemory(0x0115, 0x0F);
	// gbe_mMap->debugWriteMemory(0x0116, 0x02);

	//// DEC B
	//// Decrements the value of B by 1
	//// Final State: BC = 0x010F, Flag_N = 1, Flag_H = 0, Flag_Z = 0, AF = 0150
	// gbe_mMap->debugWriteMemory(0x0117, 0x05);

	//// LD B, u8
	//// Loads an 8 bit immediate into the register B
	//// Final State: BC = 0x690F
	// gbe_mMap->debugWriteMemory(0x0118, 0x06);
	// gbe_mMap->debugWriteMemory(0x0119, 0x69);

	//// RLCA
	//// Rotates the value of the accumulator to the left
	//// Final State: AF = 0x0200, Flag_C = 0
	// gbe_mMap->debugWriteMemory(0x011A, 0x07);

	//// Set A = 0x80 to test RLCA for C Flag

	//// RLCA
	//// TODO: Will write later when LD A instructions have been tested
	//// as that is needed to test this instruction more for flags

	//// LD (u16), SP
	//// Loads the value of the stack pointer into the memory address pointed to by the immediate
	//// Final State: Value 0xFFFE at address 0XE002
	// gbe_mMap->debugWriteMemory(0x011B, 0x08);
	// gbe_mMap->debugWriteMemory(0x011C, 0x02);
	// gbe_mMap->debugWriteMemory(0x011D, 0xE0);

	//// ADD HL, BC
	//// Adds the value of BC to HL
	//// Final State: HL = 0x6BFC
	// gbe_mMap->debugWriteMemory(0x011E, 0x09);

	//// LD BC u16
	//// Loading address 0xE003 in BC to test next instruction
	// gbe_mMap->debugWriteMemory(0x011F, 0x01);
	// gbe_mMap->debugWriteMemory(0x0120, 0x03);
	// gbe_mMap->debugWriteMemory(0x0121, 0xE0);

	//// LD A, (BC)
	//// Loads the value of the memory address pointed to by BC into the accumulator
	//// Final State: A = 0xFE, AF = 0xFE00
	// gbe_mMap->debugWriteMemory(0x0122, 0x0A);

	//// DEC BC
	//// Decrements the value of BC by 1
	//// Final State: BC = 0xE002
	// gbe_mMap->debugWriteMemory(0x0123, 0x0B);

	//// INC C
	//// Increments the value of C by 1
	//// Final State: BC = 0xE003, Flag_N = 0, Flag_H = 0, Flag_Z = 0, AF = 0xFE00
	// gbe_mMap->debugWriteMemory(0x0124, 0x0C);

	//// LD BC u16
	//// Loading address 0xE00F in BC to test H flag of INC C
	// gbe_mMap->debugWriteMemory(0x0125, 0x01);
	// gbe_mMap->debugWriteMemory(0x0126, 0x0F);
	// gbe_mMap->debugWriteMemory(0x0127, 0xE0);

	//// INC C
	//// Increments the value of C by 1
	//// Final State: BC = 0xE010, Flag_N = 0, Flag_H = 1, Flag_Z = 0, AF = 0xFE20
	// gbe_mMap->debugWriteMemory(0x0128, 0x0C);

	//// LD BC u16
	//// Loading value 0xE0FF in BC to test Z flag of INC C
	// gbe_mMap->debugWriteMemory(0x0129, 0x01);
	// gbe_mMap->debugWriteMemory(0x012A, 0xFF);
	// gbe_mMap->debugWriteMemory(0x012B, 0xE0);

	//// INC C
	//// Increments the value of C by 1
	//// Final State: BC = 0xE010, Flag_N = 0, Flag_H = 1, Flag_Z = 0, AF = 0xFE20
	// gbe_mMap->debugWriteMemory(0x012C, 0x0C);

	//// DEC C
	//// Decrements the value of C by 1
	//// Final State: BC = 0xE0FF, Flag_N = 1, Flag_H = 1, Flag_Z = 0, AF = 0xFE60
	// gbe_mMap->debugWriteMemory(0x012D, 0x0D);

	//// LD BC u16
	//// Loading value 0xE001 in BC to test H flag of DEC C
	// gbe_mMap->debugWriteMemory(0x012E, 0x01);
	// gbe_mMap->debugWriteMemory(0x012F, 0x01);
	// gbe_mMap->debugWriteMemory(0x0130, 0xE0);

	//// DEC C
	//// Decrements the value of C by 1
	//// Final State: BC = 0xE000, Flag_N = 1, Flag_H = 0, Flag_Z = 1, AF = 0xFEB0
	// gbe_mMap->debugWriteMemory(0x0131, 0x0D);

	//// LD C, u8
	//// Loads an 8 bit immediate into the register C
	//// Final State: BC = 0xE069
	// gbe_mMap->debugWriteMemory(0x0132, 0x0E);
	// gbe_mMap->debugWriteMemory(0x0133, 0x69);

	//// RRCA
	//// Rotates the value of the accumulator to the right
	//// Final State: AF = 0x7F00, Flag_C = 0
	// gbe_mMap->debugWriteMemory(0x0134, 0x0F);

	//// STOP
	//// Stops the CPU until an interrupt occurs
	//// Did a NOP for now, as this stops execution of code.
	//// TODO: Find a way to resume execution
	// gbe_mMap->debugWriteMemory(0x0135, 0x00);

	//// STOP does an unnecessary byte read
	// gbe_mMap->debugWriteMemory(0x0136, 0x00);

	//// LD DE, u16
	//// Loads an 16 bit immediate into the register DE
	//// Final State: DE = 0xFFEC
	// gbe_mMap->debugWriteMemory(0x0137, 0x11);
	// gbe_mMap->debugWriteMemory(0x0138, 0xEC);
	// gbe_mMap->debugWriteMemory(0x0139, 0xFF);

	//// LD (DE), A
	//// Loads the value of the accumulator into the memory address pointed to by DE
	//// Final State: Value 0x7F at address 0x69E0
	// gbe_mMap->debugWriteMemory(0x013A, 0x12);

	//// INC DE
	//// Increments the value of DE by 1
	//// Final State: DE = 0x69E1
	// gbe_mMap->debugWriteMemory(0x013B, 0x13);

	//// INC D
	//// Increments the value of D by 1
	//// Final State: DE = 0x6AE1, Flag_N = 0, Flag_H = 0, Flag_Z = 0, AF = 0x7F00
	// gbe_mMap->debugWriteMemory(0x013C, 0x14);

	//// DEC D
	//// Decrements the value of D by 1
	//// Final State: DE = 0x69E1, Flag_N = 1, Flag_H = 0, Flag_Z = 0, AF = 0x7F40
	// gbe_mMap->debugWriteMemory(0x013D, 0x15);

	//// LD D, u8
	//// Loads an 8 bit immediate into the register D
	//// Final State: DE = 0xE0E1
	// gbe_mMap->debugWriteMemory(0x013E, 0x16);
	// gbe_mMap->debugWriteMemory(0x013F, 0xE0);

	//// RLA
	//// Rotates the value of the accumulator to the left
	//// Final State: AF = 0xFE00, Flag_C = 0
	// gbe_mMap->debugWriteMemory(0x0140, 0x17);

	//// RLA
	//// Rotates the value of the accumulator to the left
	//// Final State: AF = 0xFC10, Flag_C = 1
	// gbe_mMap->debugWriteMemory(0x0141, 0x17);

	//// JR i8
	//// Jumps to the address at PC + i8 + 2
	//// Final State: Next instruction is selected
	//// i8 is a signed 8 bit value
	// gbe_mMap->debugWriteMemory(0x0142, 0x18);
	// gbe_mMap->debugWriteMemory(0x0143, 0x00);

	//// JR i8
	//// This one must go in an infinite loop
	// gbe_mMap->debugWriteMemory(0x0144, 0x18);
	// gbe_mMap->debugWriteMemory(0x0145, 0xFE);

	//// TODO: ADD HL, DE test after implementing and testing LD HL, u16

	//// Loading 0x0100 into DE to test LD (DE), A
	//// Final State: DE = 0x0100
	// gbe_mMap->debugWriteMemory(0x0146, 0x11);
	// gbe_mMap->debugWriteMemory(0x0147, 0x00);
	// gbe_mMap->debugWriteMemory(0x0148, 0x01);

	//// LD A, (DE)
	//// Loads the value of the memory address pointed to by DE into the accumulator
	//// Final State: AF = 0x0010
	// gbe_mMap->debugWriteMemory(0x0149, 0x1A);

	//// DEC DE
	//// Decrements the value of DE by 1
	//// Final State: DE = 0x00FF
	// gbe_mMap->debugWriteMemory(0x014A, 0x1B);

	//// INC E
	//// Increments the value of E by 1
	//// Final State: DE = 0x0100, Flag_N = 0, Flag_H = 1, Flag_Z = 1, AF = 0x00A0
	// gbe_mMap->debugWriteMemory(0x014B, 0x1C);

	//// DEC E
	//// Decrements the value of E by 1
	//// Final State: DE = 0x00FF, Flag_N = 1, Flag_H = 1, Flag_Z = 0, AF = 0x0060
	// gbe_mMap->debugWriteMemory(0x014C, 0x1D);

	//// LD E, u8
	//// Loads an 8 bit immediate into the register E
	//// Final State: DE = 0x00E0
	// gbe_mMap->debugWriteMemory(0x014D, 0x1E);
	// gbe_mMap->debugWriteMemory(0x014E, 0xE0);

	//// Loading a value into the accumulator to test RRA
	//// Final State: DE = 0x014E, AF = 0xE000
	// gbe_mMap->debugWriteMemory(0x014F, 0x11);
	// gbe_mMap->debugWriteMemory(0x0150, 0x4E);
	// gbe_mMap->debugWriteMemory(0x0151, 0x01);
	// gbe_mMap->debugWriteMemory(0x0152, 0x1A);

	//// RRA
	//// Rotates the value of the accumulator to the right
	//// Final State: AF = 0x7000, Flag_C = 0
	// gbe_mMap->debugWriteMemory(0x0153, 0x1F);

	//// Loading a value into the accumulator to test RRA
	//// Final State: DE = 0x0151, AF = 0x0100
	// gbe_mMap->debugWriteMemory(0x0154, 0x11);
	// gbe_mMap->debugWriteMemory(0x0155, 0x51);
	// gbe_mMap->debugWriteMemory(0x0156, 0x01);
	// gbe_mMap->debugWriteMemory(0x0157, 0x1A);

	//// RRA
	//// Rotates the value of the accumulator to the right
	//// Final State: AF = 0x0010, Flag_C = 1
	// gbe_mMap->debugWriteMemory(0x0158, 0x1F);

	//// JR NZ, i8
	//// Jumps to the address at PC + i8 + 2 if the zero flag is not set
	//// Final State: Next instruction is not selected as the zero flag is not set
	//// PC = 0x015C
	// gbe_mMap->debugWriteMemory(0x0159, 0x20);
	// gbe_mMap->debugWriteMemory(0x015A, 0x01);

	//// NOP
	//// This must be skipped
	// gbe_mMap->debugWriteMemory(0x015B, 0x00);

	//// Setting the zero flag to test JR NZ, i8
	//// Load 0xFF in B and INC B
	//// Final State: BC = 0x0069, AF = 0x00B0, Flag_Z = 1
	// gbe_mMap->debugWriteMemory(0x015C, 0x06);
	// gbe_mMap->debugWriteMemory(0x015D, 0xFF);
	// gbe_mMap->debugWriteMemory(0x015E, 0x04);

	//// JR NZ, i8
	//// Jumps to the address at PC + i8 + 2 if the zero flag is not set
	//// Final State: Next instruction is selected as the zero flag is set
	//// PC = 0x0161
	// gbe_mMap->debugWriteMemory(0x015F, 0x20);
	// gbe_mMap->debugWriteMemory(0x0160, 0x01);

	//// NOP
	//// This must be executed
	// gbe_mMap->debugWriteMemory(0x0161, 0x00);

	//// LD HL, u16
	//// Loads a 16 bit immediate into the register HL
	//// Final State: HL = 0xC010
	// gbe_mMap->debugWriteMemory(0x0162, 0x21);
	// gbe_mMap->debugWriteMemory(0x0163, 0x10);
	// gbe_mMap->debugWriteMemory(0x0164, 0xC0);

	//// LD (HL+), A
	//// Loads the value of the accumulator into the memory address pointed to by HL
	//// and then increments HL
	//// Final State: HL = 0xC011, AF = 0x0010, 0X00 at 0XC010
	// gbe_mMap->debugWriteMemory(0x0165, 0x22);

	//// INC HL
	//// Increments the value of HL by 1
	//// Final State: HL = 0xC012
	// gbe_mMap->debugWriteMemory(0x0166, 0x23);

	//// INC H
	//// Increments the value of H by 1
	//// Final State: HL = 0xC113, Flag_N = 0, Flag_H = 0, Flag_Z = 0, AF = 0x0010
	// gbe_mMap->debugWriteMemory(0x0167, 0x24);

	//// DEC H
	//// Decrements the value of H by 1
	//// Final State: HL = 0xC112, Flag_N = 1, Flag_H = 0, Flag_Z = 0, AF = 0x0050
	// gbe_mMap->debugWriteMemory(0x0168, 0x25);

	//// LD H, u8
	//// Loads an 8 bit immediate into the register H
	//// Final State: HL = 0xA012
	// gbe_mMap->debugWriteMemory(0x0169, 0x26);
	// gbe_mMap->debugWriteMemory(0x016A, 0xA0);

	//// DAA
	//// Adjusts the value of the accumulator to be a valid BCD value
	//// Final State: AF = 0x0010
	//// TODO: Test this. High risk opcode
	// gbe_mMap->debugWriteMemory(0x016B, 0x27);

	//// JR Z, i8
	//// Jumps to the address at PC + i8 + 2 if the zero flag is set
	//// Final State: Next instruction is selected as the zero flag is not set
	//// PC = 0x016E
	// gbe_mMap->debugWriteMemory(0x016C, 0x28);
	// gbe_mMap->debugWriteMemory(0x016D, 0x01);

	//// NOP
	//// This must be not skipped
	// gbe_mMap->debugWriteMemory(0x016E, 0x00);

	//// Setting the zero flag to test JR Z, i8
	//// Load 0xFF in B and INC B
	// gbe_mMap->debugWriteMemory(0x016F, 0x06);
	// gbe_mMap->debugWriteMemory(0x0170, 0xFF);
	// gbe_mMap->debugWriteMemory(0x0171, 0x04);

	//// JR Z, i8
	//// Jumps to the address at PC + i8 + 2 if the zero flag is set
	//// Final State: Next instruction is not selected as the zero flag is set
	//// PC = 0x0175
	// gbe_mMap->debugWriteMemory(0x0172, 0x28);
	// gbe_mMap->debugWriteMemory(0x0173, 0x01);

	//// NOP
	//// This must be skipped
	// gbe_mMap->debugWriteMemory(0x0174, 0x00);

	//// ADD HL, HL
	//// Adds the value of HL to HL
	//// Final State: HL = 0x4024, Flag_N = 0, Flag_H = 0, Flag_C = 0, AF = 0x0010
	// gbe_mMap->debugWriteMemory(0x0175, 0x29);

	//// Loading value in HL to test next opcode
	// gbe_mMap->debugWriteMemory(0x0162, 0x21);
	// gbe_mMap->debugWriteMemory(0x0163, 0x10);
	// gbe_mMap->debugWriteMemory(0x0164, 0xC0);

	//// LD A, (HL+)
	//// Loads the value of the memory address pointed to by HL into the accumulator
	//// and then increments HL
	//// Final State: HL = 0xC013, AF = 0x0010, 0X00 at 0XC011

	//// Seg fault to end using UNKOWN
	// gbe_mMap->debugWriteMemory(0x0146, 0xEB);

	executeBootROM();

	update();
}

void GBE::update()
{
	// Update function of the GBE
	// Will be called every frame
	// GB has 59.73 frames per second
	while (true)
	{
		// Execute the next instruction
		s_Cycles += gbe_cpu->executeNextInstruction();
		if ((*gbe_mMap)[0xFF02] == 0x81)
		{
			printf("%c", (*gbe_mMap)[0xFF01]);
			gbe_mMap->writeMemory(0xFF02, 0x00);
		}

		// update the DIV and TIMA timers
		gbe_cpu->updateTimers(s_Cycles);
		gbe_graphics->executePPU(s_Cycles);
		s_Cycles = 0;
		s_Cycles += gbe_cpu->performInterrupt();
		gbe_graphics->pollEvents();
	}
	// renderGraphics()
}

void GBE::executeBootROM()
{
	while (gbe_mMap->readMemory(0xFF50) == 0x00)
	{
		s_Cycles += gbe_cpu->executeNextInstruction();
		if ((*gbe_mMap)[0xFF02] == 0x81)
		{
			printf("%c", (*gbe_mMap)[0xFF01]);
			gbe_mMap->writeMemory(0xFF02, 0x00);
		}
		gbe_cpu->updateTimers(s_Cycles);
		gbe_graphics->executePPU(s_Cycles);
		s_Cycles = 0;
		s_Cycles += gbe_cpu->performInterrupt();
	}

	// Overwrite the boot ROM with first 256 bytes of game ROM
	fseek(gameROM, 0x00, SEEK_SET);
	fread(gbe_mMap->getRomBank0(), 1, 256, gameROM);
}