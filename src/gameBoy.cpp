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

void GBE::update()
{
    // Update function of the GBE
    // Will be called every frame
    // GB has 59.73 frames per second

    int cycles = 0;
    while (cycles < gbe_cpu->clockSpeedPerFrame)
    {
        // Execute the next instruction
        // cycles += gbe_cpu->executeNextInstruction();
        // updateTimers()
        // updateGraphics()
        // Do Interrupts()
    }
    // renderGraphics()
}