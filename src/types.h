#pragma once

// Data types for GBC
// Byte is 8 bits and Word is 16 bits

typedef unsigned char Byte;
typedef char SByte;
typedef unsigned short Word;
typedef signed short SWord;
typedef unsigned int color;
enum AudioRegisters {
    // NR1x
    // FF10 - FF14
    NR10,
    NR11,
    NR12,
    NR13,
    NR14,

    // NR2x
    // FF16 - FF19
    NR20, // INVALID FF15
    NR21,
    NR22,
    NR23,
    NR24,

    // NR3x
    // FF1A - FF1E
    NR30,
    NR31,
    NR32,
    NR33,
    NR34,

    // NR4x
    // FF20 - FF23
    NR40, // INVALID FF1F
    NR41,
    NR42,
    NR43,
    NR44,

    // NR5x
    // FF24 - FF26
    NR50,
    NR51,
    NR52
};