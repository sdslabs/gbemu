#include "mmap.h"

// Constructor
MemoryMap::MemoryMap()
{
	// Initialize the memory map
	// 16kb ROM bank 0
	romBank0 = new Byte[0x4000];

	// 16kb ROM bank 1
	romBank1 = new Byte[0x4000];

	// 8kb Video RAM
	videoRam = new Byte[0x2000];

	// 8kb External RAM
	externalRam = new Byte[0x2000];

	// 8kb Work RAM
	workRam = new Byte[0x2000];

	// Echo RAM is a mirror of workRam
	// But only the first 7679 bytes (0xC000 - 0xDDFF) are mirrored
	// The last 512 bytes (0xDDFF - 0xDFFF) are not mirrored
	echoRam = workRam;

	// 160 bytes OAM table
	oamTable = new Byte[0x00A0];

	// 96 bytes unused

	// 128 bytes I/O ports
	ioPorts = new Byte[0x0080];

	// 127 bytes High RAM
	highRam = new Byte[0x007F];

	// 1 byte Interrupt Enable Register
	interruptEnableRegister = new Byte;
}

Byte* MemoryMap::getRomBank0()
{
	return romBank0;
}

Byte* MemoryMap::getRomBank1()
{
	return romBank1;
}

Byte* MemoryMap::getVideoRam()
{
	return videoRam;
}

Byte* MemoryMap::getExternalRam()
{
	return externalRam;
}

Byte* MemoryMap::getWorkRam()
{
	return workRam;
}

Byte* MemoryMap::getEchoRam()
{
	return echoRam;
}

Byte* MemoryMap::getOamTable()
{
	return oamTable;
}

Byte* MemoryMap::getIoPorts()
{
	return ioPorts;
}

Byte* MemoryMap::getHighRam()
{
	return highRam;
}

Byte* MemoryMap::getInterruptEnableRegister()
{
	return interruptEnableRegister;
}

// Write to memory
bool MemoryMap::writeMemory(Word address, Byte value)
{
	if (address < 0x8000)
	{
		printf("Writing to ROM is not allowed");
		return false;
	}
	else if (address < 0xA000)
	{
		// Write to Video RAM
		videoRam[address - 0x8000] = value;
	}
	else if (address < 0xC000)
	{
		// Write to External RAM
		externalRam[address - 0xA000] = value;
	}
	else if (address < 0xE000)
	{
		// Write to Work RAM
		workRam[address - 0xC000] = value;
	}
	else if (address < 0xFE00)
	{
		// Write to Echo RAM
		echoRam[address - 0xE000] = value;
	}
	else if (address < 0xFEA0)
	{
		// Write to OAM Table
		oamTable[address - 0xFE00] = value;
	}
	else if (address < 0xFF00)
	{
		// Write to unused memory
		printf("Writing to unused memory is not allowed");
		return false;
	}
	else if (address < 0xFF80)
	{
		// Write to I/O Ports
		ioPorts[address - 0xFF00] = value;
	}
	else if (address < 0xFFFF)
	{
		// Write to High RAM
		highRam[address - 0xFF80] = value;
	}
	else if (address == 0xFFFF)
	{
		// Write to Interrupt Enable Register
		*interruptEnableRegister = value;
	}
	else
	{
		printf("Invalid address");
		return false;
	}

	return true;
}