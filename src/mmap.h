#pragma once
#include "types.h"
#include <stdio.h>

// The Memory Map for GBE
// Pulled from https://gbdev.io/pandocs/Memory_Map.html

class MemoryMap
{
private:
	Byte mbcMode;

	// First ROM Bank
	// 16 KB 0x0000 - 0x3FFF
	// Contains the first 16 KB of the ROM
	Byte* romBank0;

	// Second ROM Bank
	// 16 KB 0x4000 - 0x7FFF
	// Contains the second 16 KB of the ROM which can be switched
	Byte* romBank1;

	// Video RAM
	// 8 KB 0x8000 - 0x9FFF
	Byte* videoRam;

	// External RAM
	// 8 KB 0xA000 - 0xBFFF
	// Typically a ROM + SRAM or an MBC
	Byte* externalRam;

	// Work RAM Bank
	// 8 KB 0xC000 - 0xDFFF
	// CPU can write to these bank
	// I have picked only 1 chunk instead of two, makes it easy for echoRAM
	Byte* workRam;

	// Echo RAM
	// 8 KB 0xE000 - 0xFDFF
	// Mirror of workRAM
	Byte* echoRam;

	// Sprite Attribute Table
	// 160 Bytes 0xFE00 - 0xFE9F
	Byte* oamTable;

	// Unusable Memory
	// 96 Bytes 0xFEA0 - 0xFEFF
	// Byte unused[0x0060];

	// I/O Ports
	// 128 Bytes 0xFF00 - 0xFF7F
	Byte* ioPorts;

	// High RAM
	// 127 Bytes 0xFF80 - 0xFFFE
	Byte* highRam;

	// Interrupt Enable Register
	// 1 Byte 0xFFFF
	Byte* interruptEnableRegister;

	// The Joypad Input
	// stays in the I/O Ports at 0xFF00
	Byte* reg_JOYP;

	// The divider register
	// stays in the I/O Ports at 0xFF04
	Byte* reg_DIV;

	// The timer counter
	// stays in the I/O Ports at 0xFF05
	// Increments at the frequency specified at 0xFF07
	// Raises an interrupt when overflown and resets to value at 0xFF06
	Byte* reg_TIMA;

	// The timer modulo
	// stays in the I/O Ports at 0xFF06
	// Resets TIMA to its value when TIMA overflows
	// More info at https://gbdev.io/pandocs/Timer_and_Divider_Registers.html
	Byte* reg_TMA;

	// The timer control
	// stays in I/O Ports at 0xFF07
	// Specifies frequency at which to update TIMA and enable timer
	Byte* reg_TAC;

	// Interrupt Flag
	// Stays in the I/O Ports at 0xFF0F
	// Signals which interrupt must take place
	Byte* reg_IF;

	// The LCD Control Register
	// Stays in the I/O Ports at 0xFF40
	Byte* reg_LCDC;

	// The SCX Register
	// Stays in the I/O Ports at 0xFF43
	Byte* reg_SCX;

	// The SCY Register
	// Stays in the I/O Ports at 0xFF42
	Byte* reg_SCY;

	// The BGP Register
	// Stays in the I/O Ports at 0xFF47
	Byte* reg_BGP;

	// The LY Register
	// Stays in the I/O Ports at 0xFF44
	Byte* reg_LY;

	// The LYC Register
	// Stays in the I/O Ports at 0xFF45
	Byte* reg_LYC;

	// The STAT Register
	// Stays in the I/O Ports at 0xFF41
	Byte* reg_STAT;

	// The WY Register
	// Stays in the I/O Ports at 0xFF4A
	Byte* reg_WY;

	// The WX Register
	// Stays in the I/O Ports at 0xFF4B
	Byte* reg_WX;

public:
	// Constructor
	MemoryMap();

	// Destructor
	~MemoryMap();

	// Returns the ROM Bank 0
	Byte* getRomBank0() const { return romBank0; }

	// Returns the ROM Bank 1
	Byte* getRomBank1() const { return romBank1; }

	// Returns the Video RAM
	Byte* getVideoRam() const { return videoRam; }

	// Returns the External RAM
	Byte* getExternalRam() const { return externalRam; }

	// Returns the Work RAM
	Byte* getWorkRam() const { return workRam; }

	// Returns the Echo RAM
	Byte* getEchoRam() const { return echoRam; }

	// Returns the OAM Table
	Byte* getOamTable() const { return oamTable; }

	// Returns the I/O Ports
	Byte* getIoPorts() const { return ioPorts; }

	// Returns the High RAM
	Byte* getHighRam() const { return highRam; }

	// Returns the Interrupt Enable Register
	Byte* getInterruptEnableRegister() { return interruptEnableRegister; }

	// Writes a byte to the memory address
	bool writeMemory(Word address, Byte value);
	void debugWriteMemory(Word address, Byte value);

	// Reads a byte from the memory address
	Byte readMemory(Word address);

	// Operator overload for the readMemory function
	Byte operator[](Word address);

	// increments the divider register
	void updateDividerRegister() { (*reg_DIV)++; }

	// gets the reg_TAC
	Byte getRegTAC() { return *reg_TAC; }

	// gets the reg_TMA
	Byte getRegTMA() { return *reg_TMA; }

	// gets the reg_TIMA
	Byte getRegTIMA() { return *reg_TIMA; }

	// gets the reg_IF
	Byte getRegIF() { return *reg_IF; }

	// gets the reg_IE
	Byte getRegIE() { return *interruptEnableRegister; }

	// gets the reg_LCDC
	Byte getRegLCDC() { return *reg_LCDC; }

	// gets the reg_SCX
	Byte getRegSCX() { return *reg_SCX; }

	// gets the reg_SCY
	Byte getRegSCY() { return *reg_SCY; }

	// gets the reg_BGP
	Byte getRegBGP() { return *reg_BGP; }

	// gets the reg_LY
	Byte getRegLY() { return *reg_LY; }

	// gets the reg_LYC
	Byte getRegLYC() { return *reg_LYC; }

	// gets the reg_STAT
	Byte getRegSTAT() { return *reg_STAT; }

	// gets the reg_WY
	Byte getRegWY() { return *reg_WY; }

	// gets the reg_WX
	Byte getRegWX() { return *reg_WX; }

	// sets the reg_TIMA
	void setRegTIMA(Byte value) { *reg_TIMA = value; }

	// sets the reg_IF to request an interrupt
	void setRegIF(Byte value) { *reg_IF |= value; }

	// sets the reg_LY
	void setRegLY(Byte value) { *reg_LY = value; }

	// sets the reg_STAT
	void setRegSTAT(Byte value) { *reg_STAT = value; }
};