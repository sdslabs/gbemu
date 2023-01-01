#include "types.h"
#include "cpu.h"
#include "gameBoy.h"

GBE::GBE()
{   
    // Initialize the CPU
    gbe_cpu = new CPU();
}

CPU* GBE::getCPU()
{
    // Getter of CPU
    return gbe_cpu;
}