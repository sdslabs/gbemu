#include "mmap.h"

// Constructor
MemoryMap::MemoryMap() {
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