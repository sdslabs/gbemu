#include "mmap.h"
#include <cstring>

// Constructor
MemoryMap::MemoryMap()
{
	// Initialize the memory map
	// 16kb ROM bank 0
	romBank0 = new Byte[0x4000];
	memset(romBank0, 0x00, 0x4000);

	// 16kb ROM bank 1
	romBank1 = new Byte[0x4000];
	memset(romBank1, 0x00, 0x4000);

	// 8kb Video RAM
	videoRam = new Byte[0x2000];
	memset(videoRam, 0x00, 0x2000);

	// 8kb External RAM
	externalRam = new Byte[0x2000];
	memset(externalRam, 0x00, 0x2000);

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

	mbcMode = 0x0;

	romSize = 0x0;

	ramSize = 0x0;

	ramEnable = 0;

	romBankNumber = 0;

	ramBankNumber = 0;

	bankingModeSelect = 0;

	ramExistenceMask = 0;

	romBankNumberMask = 0;

	ramBankNumberMaskForRom = 0;

	ramBankNumberMaskForRam = 0;
}

// Write to memory
// TODO: Make emulation memory secure
bool MemoryMap::writeMemory(Word address, Byte value)
{
	if (address < 0x2000)
	{
		// If values is 0x0A then external RAM is enabled, else it is disabled
		if (value == 0x0A)
		{
			ramEnable = true;
		}
		else
		{
			ramEnable = false;
		}
	}
	else if (address < 0x4000)
	{
		// Decide ROM Bank Number
		romBankNumber = (value & 0b11111);
		bankRom();
	}
	else if (address < 0x6000)
	{
		// Decide RAM Bank Number
		ramBankNumber = (value & 0b11);
		bankRom();
		bankRam();
	}
	else if (address < 0x8000)
	{
		// Decide Banking Mode Select
		bankingModeSelect = (value & 0b1);
		bankRom();
		bankRam();
	}
	else if (address < 0xA000)
	{
		// Write to Video RAM
		videoRam[address - 0x8000] = value;
	}
	else if (address < 0xC000)
	{
		// Write to External RAM if external RAM has been enabled
		if (ramEnable)
		{
			externalRam[address - 0xA000] = value;
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
		return romBank0[address];
	}
	else if (address < 0x8000)
	{
		// Read from ROM bank 1
		return romBank1[address - 0x4000];
	}
	else if (address < 0xA000)
	{
		// Read from Video RAM
		return videoRam[address - 0x8000];
	}
	else if (address < 0xC000)
	{
		// Read from External RAM
		return externalRam[address - 0xA000];
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
	fread(romBank1, 1, 16384, romFile);

	// MBC

	// Check 0x147 for MBC mode
	mbcMode = romBank0[0x147];

	// Check 0x148 for ROM size
	romSize = romBank0[0x148];

	// Check 0x149 for RAM size
	ramSize = romBank0[0x149];

	// Seek to begining of ROM
	fseek(romFile, 0x00, SEEK_SET);

	// Get the ROM banks
	romBankList = new Byte[2 << romSize][0x4000];
	for (int i = 0; i < (2 << romSize); ++i)
	{
		memset(romBankList[i], 0x00, 0x4000);
	}
	for (int i = 0; i < (2 << romSize); ++i)
	{
		fread(romBankList[i], 1, 16384, romFile);
	}

	// Set RAM banks
	int ramBanks = 0;
	switch (ramSize)
	{
	case 0:
		ramBanks = 0;
		break;
	case 2:
		ramBanks = 1;
		break;
	case 3:
		ramBanks = 4;
		break;
	case 4:
		ramBanks = 16;
		break;
	case 5:
		ramBanks = 8;
		break;
	}
	ramBankList = new Byte[ramBanks][0x2000];
	for (int i = 0; i < ramBanks; ++i)
	{
		memset(ramBankList[i], 0x00, 0x2000);
	}
	if (ramBanks > 0)
	{
		externalRam = ramBankList[0];
	}

	// Set the RAM Existence Mask
	// Tells if External RAM is available in the Cartridge
	if ((mbcMode == 2 or mbcMode == 3) and ramBanks > 0)
	{
		ramExistenceMask = 1;
	}

	// Set the ROM Bank Number Mask
	// Tells the useful bits in the ROM Bank number depending on ROM size
	romBankNumberMask = ((2 << romSize) < 0b100000) ? (2 << romSize) : 0b100000;
	romBankNumberMask -= 1;

	// Set the RAM Bank Number Mask for ROM
	// Tells the useful bits in the RAM Bank number for ROM depending on ROM size
	if (romSize > 4)
	{
		ramBankNumberMaskForRom = ((1 << (romSize - 4)) < 0b100) ? (1 << (romSize - 4)) : 0b100;
		ramBankNumberMaskForRom -= 1;
	}

	// Set the RAM Bank Number Mask for RAM
	// Tells the useful bits in the RAM Bank number for RAM depending on RAM size
	if (ramBanks > 0)
	{
		ramBankNumberMaskForRam = (ramBanks < 0b100) ? ramBanks : 0b100;
		ramBankNumberMaskForRam -= 1;
	}
}

void MemoryMap::unloadBootRom()
{
	fseek(romFile, 0x00, SEEK_SET);
	fread(romBank0, 1, 256, romFile);
}

void MemoryMap::bankRom()
{

	Byte completeBankNumber = romBankNumber;

	if (completeBankNumber == 0)
	{
		completeBankNumber += 1;
	}

	completeBankNumber &= romBankNumberMask;
	completeBankNumber |= ((ramBankNumber & ramBankNumberMaskForRom) << 5);

	romBank0 = romBankList[((ramBankNumber & ramBankNumberMaskForRom) << 5) & (bankingModeSelect * 0b1100000)];
	romBank1 = romBankList[completeBankNumber];
}

void MemoryMap::bankRam()
{
	if (ramExistenceMask)
	{
		externalRam = ramBankList[(ramBankNumber & ramBankNumberMaskForRam) & (bankingModeSelect * 0b11)];
	}
}