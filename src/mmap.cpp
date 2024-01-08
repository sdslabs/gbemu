#include "mmap.h"
#include <cstring>

// Constructor
MemoryMap::MemoryMap()
{
	romSize = 0x8000;
	ramSize = 0x2000;

	// Initialize the memory map
	// 16kb ROM bank 0
	romBank0 = new Byte[0x4000];
	memset(romBank0, 0x00, 0x4000);

	// 8kb External RAM
	videoRam = new Byte[0x2000];
	memset(videoRam, 0x00, 0x2000);

	// 8kb Work RAM
	workRam = new Byte[0x2000];
	memset(workRam, 0x00, 0x2000);

	// Echo RAM is a mirror of workRam
	// But only the first 7679 bytes (0xC000 - 0xDDFF) are mirrored
	// The last 512 bytes (0xDDFF - 0xDFFF) are not mirrored
	echoRam = workRam;

	// 160 bytes OAM table
	oamTable = new Byte[0x00A0];
	memset(oamTable, 0x00, 0x00A0);

	// 96 bytes unused

	// 128 bytes I/O ports
	ioPorts = new Byte[0x0080];
	memset(ioPorts, 0x00, 0x0080);

	// 127 bytes High RAM
	highRam = new Byte[0x007F];
	memset(highRam, 0x00, 0x007F);

	// MBC mode
	romMBCMode = 0x0;

	// ROM Bank Number
	romBankNumber = 0x1;

	// RAM Bank Number
	ramBankNumber = 0x0;

	// Enable RAM Flag
	enableRAM = 0x0;

	// ROM/RAM Mode Select
	romRAMModeSelect = 0x0;

	// 1 byte Interrupt Enable Register
	interruptEnableRegister = new Byte;

	// Joypad Input at 0xFF00
	reg_JOYP = ioPorts + 0x00;

	// DIV at 0xFF04
	reg_DIV = ioPorts + 0x04;

	// TIMA at 0xFF05
	reg_TIMA = ioPorts + 0x05;

	// TMA at 0xFF06
	reg_TMA = ioPorts + 0x06;

	// TAC at 0xFF07
	reg_TAC = ioPorts + 0x07;

	// IF at 0xFF0F
	reg_IF = ioPorts + 0x0F;

	// LCDC at 0xFF40
	reg_LCDC = ioPorts + 0x40;

	// SCX at 0xFF43
	reg_SCX = ioPorts + 0x43;

	// SCY at 0xFF42
	reg_SCY = ioPorts + 0x42;

	// BGP at 0xFF47
	reg_BGP = ioPorts + 0x47;

	// OBP0 at 0xFF48
	reg_OBP0 = ioPorts + 0x48;

	// OBP1 at 0xFF49
	reg_OBP1 = ioPorts + 0x49;

	// LY at 0xFF44
	reg_LY = ioPorts + 0x44;

	// LYC at 0xFF45
	reg_LYC = ioPorts + 0x45;

	// STAT at 0xFF41
	reg_STAT = ioPorts + 0x41;

	// WY at 0xFF4A
	reg_WY = ioPorts + 0x4A;

	// WX at 0xFF4B
	reg_WX = ioPorts + 0x4B;

	joyPadState = new Byte;
	*joyPadState = 0xFF;

	bootRomFile = nullptr;
	romFile = nullptr;
}

// Check writes to ROM
void MemoryMap::checkRomWrite(Word address, Byte value)
{
	switch (romMBCMode)
	{
	case MBC0:
		printf("Writing to ROM is not allowed! Write attempted at %04X", address);
		break;
	case MBC1:
	{
		if (address < 0x2000)
		{
			// Enable/Disable RAM
			if ((value & 0x0F) == 0x0A)
				enableRAM = 0xFF;
			else
				enableRAM = 0x00;
		}
		else if (address < 0x4000)
		{
			romBankNumber &= 0x60;
			romBankNumber |= (value & 0x1F);
		}
		else if (address < 0x6000)
		{
			if (romRAMModeSelect)
			{
				ramBankNumber = value & 0x03;
				romBankNumber &= 0x1F;
				romBankNumber |= ((value & 0x03) << 5);
			}
		}
		else if (address < 0x8000)
		{
			romRAMModeSelect = value & 0x01;
		}
		else
		{
			printf("Invalid address");
		}
		break;
	}
	default:
		printf("Invalid MBC mode");
		break;
	}
}

// Write to memory
bool MemoryMap::writeMemory(Word address, Byte value)
{
	if (address < 0x8000)
	{
		checkRomWrite(address, value);
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
		switch (romMBCMode)
		{
		case MBC0:
			externalRam[address - 0xA000] = value;
			break;
		case MBC1:
			if (enableRAM && romRAMModeSelect && ((ramBankNumber * 0x2000) < ramSize))
				externalRam[address - 0xA000 + (ramBankNumber * 0x2000)] = value;
			else if (enableRAM)
				externalRam[address - 0xA000] = value;
			break;
		default:
			externalRam[address - 0xA000] = value;
			break;
		}
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
		// Check for reg_DIV write quirk
		// Writes to DIV reset the DIV to 0
		// else write to I/O ports
		if (address == 0xFF04)
			*reg_DIV = 0x00;
		// Check for DMA transfer
		// Writing a loop instead of std::copy
		// as memoury is not a single unit
		// in our architecture
		else if (address == 0xFF46)
		{
			Word val = value;
			val = val << 8;
			for (Word i = 0; i < 0xA0; i++)
				oamTable[i] = readMemory(val + i);
			ioPorts[address - 0xFF00] = value;
		}
		else if (address == 0xFF44)
			*reg_LY = 0x00;
		else if (address == 0xFF00)
		{
			readInput(value);
		}
		//if (value != 0xFF)
		//printf("0x%02x\n", ioPorts[0]);}
		else
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

void MemoryMap::debugWriteMemory(Word address, Byte value)
{
	romBank0[address] = value;
}

Byte MemoryMap::readMemory(Word address)
{
	if (address < 0x4000)
	{
		// Read from ROM bank 0
		switch (romMBCMode)
		{
		case MBC0:
			return romBank0[address];
		case MBC1:
			if (romRAMModeSelect && (romBankNumber & 0x60) && ((romBankNumber * 0x4000) < romSize) && !(romBankNumber & 0x1F))
				return romBank1[address + (((romBankNumber & 0x60) - 1) * 0x4000)];
			else
				return romBank0[address];
		default:
			return romBank0[address];
		}
	}
	else if (address < 0x8000)
	{
		// Read from ROM bank 1
		switch (romMBCMode)
		{
		case MBC0:
			return romBank1[address - 0x4000];
		case MBC1:
			if (!(romBankNumber & 0x1F))
				return romBank1[(address - 0x4000) + (romBankNumber * 0x4000)];
			else if ((romBankNumber * 0x4000) < romSize)
				return romBank1[(address - 0x4000) + ((romBankNumber - 1) * 0x4000)];
			else
				return romBank1[(address - 0x4000) + (((romBankNumber & 0x1F) - 1) * 0x4000)];
		default:
			return romBank1[address - 0x4000];
		}
	}
	else if (address < 0xA000)
	{
		// Read from Video RAM
		return videoRam[address - 0x8000];
	}
	else if (address < 0xC000)
	{
		// Read from External RAM
		switch (romMBCMode)
		{
		case MBC0:
			return externalRam[address - 0xA000];
		case MBC1:
			if (enableRAM) {
				if (romRAMModeSelect && ((ramBankNumber * 0x2000) < ramSize))
					return externalRam[address - 0xA000 + (ramBankNumber * 0x2000)];
				else
					return externalRam[address - 0xA000];
			}
			else
				return 0xFF;
		default:
			return externalRam[address - 0xA000];
		}
	}
	else if (address < 0xE000)
	{
		// Read from Work RAM
		return workRam[address - 0xC000];
	}
	else if (address < 0xFE00)
	{
		// Read from Echo RAM
		return echoRam[address - 0xE000];
	}
	else if (address < 0xFEA0)
	{
		// Read from OAM Table
		return oamTable[address - 0xFE00];
	}
	else if (address < 0xFF00)
	{
		// Read from unused memory
		// TODO: Check https://gbdev.io/pandocs/Memory_Map.html#fea0-feff-range
		return 0;
	}
	else if (address < 0xFF80)
	{
		// Read from I/O Ports
		return ioPorts[address - 0xFF00];
	}
	else if (address < 0xFFFF)
	{
		// Read from High RAM
		return highRam[address - 0xFF80];
	}
	else if (address == 0xFFFF)
	{
		// Read from Interrupt Enable Register
		return *interruptEnableRegister;
	}
	else
	{
		printf("Invalid address");
		return 0;
	}
}

Byte MemoryMap::operator[](Word address)
{
	return MemoryMap::readMemory(address);
}

void MemoryMap::readInput(Byte value)
{
	ioPorts[0] = (ioPorts[0] & 0xCF) | (value & 0x30);

	Byte current = ioPorts[0] & 0xF0;

	switch (current & 0x30)
	{
	case 0x10:
		current = 0xD0;
		current |= (((*joyPadState) >> 4) & 0x0F);
		break;
	case 0x20:
		current = 0xE0;
		current |= ((*joyPadState) & 0x0F);
		break;
	case 0x30:
		current = 0xF0;
		current |= 0x0F;
		break;
	}

	if ((ioPorts[0] & (~current) & 0x0F) != 0)
		(*reg_IF) |= 0x10;

	ioPorts[0] = current;
}

void MemoryMap::mapRom()
{
	// Load the Boot ROM
	// Into the first 0x100 bytes
	fread(romBank0, 1, 256, bootRomFile);

	// Load Game ROM in Bank 0
	// After offsetting for Boot ROM first
	fseek(romFile, 0x100, SEEK_SET);

	fread(romBank0 + 0x100, 1, 16128, romFile);

	// Find the size of the ROM and RAM
	// https://gbdev.io/pandocs/The_Cartridge_Header.html#0148--rom-size
	romSize = 0x8000 * (1 << romBank0[0x148]);
	switch (romBank0[0x149])
	{
	case 0x0:
		ramSize = 0x0;
		break;
	case 0x1:
		ramSize = 0x800;
		break;
	case 0x2:
		ramSize = 0x2000;
		break;
	case 0x3:
		ramSize = 0x8000;
		break;
	case 0x4:
		ramSize = 0x20000;
		break;
	case 0x5:
		ramSize = 0x10000;
		break;
	}

	// Allocate memory for ROM Bank 1
	romBank1 = new Byte[romSize - 0x4000];

	// Load the ROM Bank 1
	fseek(romFile, 0x4000, SEEK_SET);
	fread(romBank1, 1, romSize - 0x4000, romFile);

	// Check 0x147 for MBC mode
	romMBCMode = romBank0[0x147];
	switch (romMBCMode)
	{
	case 0x00:
		// No MBC
		romMBCMode = MBC0;
		externalRam = new Byte[0x2000];
		memset(externalRam, 0x00, 0x2000);
		break;
	case 0x01:
	case 0x02:
	case 0x03:
		// MBC1
		romMBCMode = MBC1;
		externalRam = new Byte[ramSize];
		memset(externalRam, 0x00, ramSize);
		break;
	default:
		printf("Invalid MBC mode");
		break;
	}
}

void MemoryMap::unloadBootRom()
{
	fseek(romFile, 0x00, SEEK_SET);
	fread(romBank0, 1, 256, romFile);
}