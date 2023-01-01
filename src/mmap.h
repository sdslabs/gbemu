#pragma once
#include "types.h"
#include <stdio.h>

// The Memory Map for GBE
// Pulled from https://gbdev.io/pandocs/Memory_Map.html

class MemoryMap
{
private:
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

public:
	// Constructor
	MemoryMap();

	// Destructor
	~MemoryMap();

	// Returns the ROM Bank 0
	Byte* getRomBank0();

	// Returns the ROM Bank 1
	Byte* getRomBank1();

	// Returns the Video RAM
	Byte* getVideoRam();

	// Returns the External RAM
	Byte* getExternalRam();

	// Returns the Work RAM
	Byte* getWorkRam();

	// Returns the Echo RAM
	Byte* getEchoRam();

	// Returns the OAM Table
	Byte* getOamTable();

	// Returns the I/O Ports
	Byte* getIoPorts();

	// Returns the High RAM
	Byte* getHighRam();

	// Returns the Interrupt Enable Register
	Byte* getInterruptEnableRegister();

	bool writeMemory(Word address, Byte value);
};