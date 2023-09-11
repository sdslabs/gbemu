#pragma once
#include "types.h"
#include "mmap.h"
#include <stdio.h>
#include <SDL.h>

class PulseChannel
{
private:
    // https://gbdev.io/pandocs/Audio_Registers.html
    
    // NRx0 
    Byte sweepPace;
    Byte sweepChange;
    Byte sweepSlope;

    // NRx1 
    Byte waveDuty;
    Byte lengthTimer;

    // NRx2
    Byte envelopeVolume;
    Byte envelopeDirection;
    Byte envelopeSweepPace;

    // Nrx3
    // needs 11 bits - 3 bits from NRX4 + 8 bits from NRx3  
    Word periodValue; 

    // NRx4
    bool trigger;
    bool soundLengthEnable;

};

class WaveChannel
{
private:
    // NRx0
    bool enable;

    // NRx1
    Byte lengthTimer;

    // NRx2
    Byte outputLevel;

    // Nrx3
    // needs 11 bits - 3 bits from NRX4 + 8 bits from NRx3  
    Word periodValue; 

    // NRx4
    bool trigger;
    bool soundLengthEnable;

    // Wave RAM
    Byte wRAM[16];
};

class NoiseChannel
{
private:
    // NRx1
    Byte lengthTimer;

    // NRx2
    Byte envelopeVolume;
    Byte envelopeDirection;
    Byte envelopeSweepPace;

    // NRx3
    Byte clockShift;
    Byte LFSRWidth;
    Byte clockDivider;

    // NRx4
    bool trigger;
    bool soundLengthEnable;

};


class APU
{
private:
    // SDL Audio
    // https://documentation.help/SDL/guideaudioexamples.html
    SDL_AudioSpec wanted, obtained;
    SDL_AudioDeviceID audioDeviceID;

    static Uint8 *audio_chunk;
    static Uint32 audio_len;
    static Uint8 *audio_pos;

    // memory map     
    MemoryMap* mMap;

    // Audio Controllers
    bool enableOutput;
    bool channelEnable[4];

    Byte soundPann;

    bool enableVINLeft;
    bool enableVINRight;

    Byte volumeLeft;
    Byte volumeRight;
    
    //Audio Channels
    PulseChannel* channel1;
    PulseChannel* channel2;

    WaveChannel* channel3;
    
    NoiseChannel* channel4;


public:
    APU();
    bool init();
    void setMemoryMap(MemoryMap* m){ mMap = m; }
    void test();

};
