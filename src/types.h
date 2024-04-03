#pragma once

// Data types for GBC
// Byte is 8 bits and Word is 16 bits

typedef unsigned char Byte;
typedef char SByte;
typedef unsigned short Word;
typedef signed short SWord;
typedef unsigned int color;
#define BIT7 0b10000000
#define BIT6 0b01000000
#define BIT5 0b00100000
#define BIT4 0b00010000
#define BIT3 0b00001000
#define BIT2 0b00000100
#define BIT1 0b00000010
#define BIT0 0b00000001

enum AudioRegisters {
    // NR1x
    // FF10 - FF14
    NR10 = 0x0,
    NR11 = 0x1,
    NR12 = 0x2,
    NR13 = 0x3,
    NR14 = 0x4,

    // NR2x
    // FF16 - FF19
    NR21 = 0x6,
    NR22 = 0x7,
    NR23 = 0x8,
    NR24 = 0x9,

    // NR3x
    // FF1A - FF1E
    NR30 = 0xA,
    NR31 = 0xB,
    NR32 = 0xC,
    NR33 = 0xD,
    NR34 = 0xE,

    // NR4x
    // FF20 - FF23
    NR41 = 0x10,
    NR42 = 0x11,
    NR43 = 0x12,
    NR44 = 0x13,

    // NR5x
    // FF24 - FF26
    NR50 = 0x14,
    NR51 = 0x15,
    NR52 = 0x16
};


