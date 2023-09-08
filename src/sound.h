#pragma once
#include "types.h"
#include "mmap.h"
#include <stdio.h>
#include <SDL.h>


class MasterController
{

};

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

};

class NoiseChannel
{
private:

};


class APU
{
private:
    // memory map     
    MemoryMap* mMap;

    PulseChannel* channel1;
    PulseChannel* channel2;

    // WaveChannel* channel3;
    
    // NoiseChannel* channel4;

public:
    APU();
    void setMemoryMap(MemoryMap* m){ mMap = m; }
    void test();
};
