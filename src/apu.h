#pragma once
#include "types.h"
#include "mmap.h"

class APU
{
private:
	MemoryMap* apu_mMap;

public:
	APU(MemoryMap* mmap);
	void checkDAC();
};