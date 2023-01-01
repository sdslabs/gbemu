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

// Write to memory
// TODO: Make emulation memory secure
bool MemoryMap::writeMemory(Word address, Byte value) {
    if (address < 0x8000) {
        printf("Writing to ROM is not allowed");
        return false;
    } else if (address < 0xA000) {
        // Write to Video RAM
        videoRam[address - 0x8000] = value;
    } else if (address < 0xC000) {
        // Write to External RAM
        externalRam[address - 0xA000] = value;
    } else if (address < 0xE000) {
        // Write to Work RAM
        workRam[address - 0xC000] = value;
    } else if (address < 0xFE00) {
        // Write to Echo RAM
        echoRam[address - 0xE000] = value;
    } else if (address < 0xFEA0) {
        // Write to OAM Table
        oamTable[address - 0xFE00] = value;
    } else if (address < 0xFF00) {
        // Write to unused memory
        printf("Writing to unused memory is not allowed");
        return false;
    } else if (address < 0xFF80) {
        // Write to I/O Ports
        ioPorts[address - 0xFF00] = value;
    } else if (address < 0xFFFF) {
        // Write to High RAM
        highRam[address - 0xFF80] = value;
    } else if (address == 0xFFFF) {
        // Write to Interrupt Enable Register
        *interruptEnableRegister = value;
    } else {
        printf("Invalid address");
        return false;
    }

    return true;
}

Byte MemoryMap::readMemory(Word address) {
    if (address < 0x4000) {
        // Read from ROM bank 0
        return romBank0[address];
    } else if (address < 0x8000) {
        // Read from ROM bank 1
        return romBank1[address - 0x4000];
    } else if (address < 0xA000) {
        // Read from Video RAM
        return videoRam[address - 0x8000];
    } else if (address < 0xC000) {
        // Read from External RAM
        return externalRam[address - 0xA000];
    } else if (address < 0xE000) {
        // Read from Work RAM
        return workRam[address - 0xC000];
    } else if (address < 0xFE00) {
        // Read from Echo RAM
        return echoRam[address - 0xE000];
    } else if (address < 0xFEA0) {
        // Read from OAM Table
        return oamTable[address - 0xFE00];
    } else if (address < 0xFF00) {
        // Read from unused memory
        // TODO: Check https://gbdev.io/pandocs/Memory_Map.html#fea0-feff-range
        return 0;
    } else if (address < 0xFF80) {
        // Read from I/O Ports
        return ioPorts[address - 0xFF00];
    } else if (address < 0xFFFF) {
        // Read from High RAM
        return highRam[address - 0xFF80];
    } else if (address == 0xFFFF) {
        // Read from Interrupt Enable Register
        return *interruptEnableRegister;
    } else {
        printf("Invalid address");
        return 0;
    }
}