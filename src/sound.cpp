#include "sound.h"
#include "types.h"

APU::APU()
{
    mMap = nullptr;

}

void APU::test()
{
    for (int i = 0; i < 0x20; i++)
    {
    printf("------------------------ at register FF%d: %d\n\n", i, mMap->readMemory(0xff10 + i) );
    }
    
}