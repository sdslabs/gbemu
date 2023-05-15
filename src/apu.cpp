#include "apu.h"

APU::APU(MemoryMap* mmap)
{
	apu_mMap = mmap;
}

void APU::checkDAC()
{
	//(apu_mMap->getRegNR52() & 0x80) ? printf("Sound is on\n") : printf("Sound is off\n");
}