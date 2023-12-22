#pragma once
#include "types.h"
#include "mmap.h"
#include <stdio.h>
#include <SDL.h>

class PulseChannel
{
private:
    // https://gbdev.io/pandocs/Audio_Registers.html
    
    // The address of register NRx1; x can be 1 or 2 
    Word regAddr = 0;

    // NRx0, NRx1, NRx2, NRx3, NRx4
	Word NR[5];

    MemoryMap* mMap;

    bool sweepPresent;

    bool enable;

    Byte volume;
    int frequencyTimer;

    // NRx0 
    Byte sweepPace;
    Byte sweepPaceClock;
    Byte sweepChange;
    Byte sweepSlope;


    // NRx1 
    Byte waveDuty;
    Byte lengthTimer;
    Byte lengthTimerClock;

    // NRx2
    Byte envelopeVolume;
    Byte envelopeDirection;
    Byte envelopeSweepPace;
    Byte envelopeSweepPaceClock;

    // Nrx3
    // needs 11 bits - 3 bits from NRX4 + 8 bits from NRx3  
    Word periodValue; 
    Word periodValueTemp; 
    Word periodValueClock; 

    // NRx4
    bool trigger;
    bool soundLengthEnable;

    // Distribution of wave-form over range of 8 cycles
    // https://gbdev.io/pandocs/Audio_Registers.html#ff11--nr11-channel-1-length-timer--duty-cycle
    bool waveDutyTab[4][8] = 
    {
        {1, 1, 1, 1, 1, 1, 1, 0},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {0, 1, 1, 1, 1, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 1}
    };

    Byte WaveDutyCounter;



public:
    PulseChannel();
    bool init(Byte channelNum);
    void run(Byte rateDIV);
    void step(int cycles);
    void enableAndLoad();
    void setMemoryMap(MemoryMap* m){  mMap = m;}
    bool checkTrigger();
    void takeSample();
    Byte getVolume();
    bool checkEnable();
    void readPeriodValue();
};

class WaveChannel
{
private:

    Word NR[5];
    Word registerAddress=0xFF1A;

    MemoryMap* mMap; 
    Byte index;
    Byte volume;
    Byte outVolume;
    int frequencyTimer;

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
    Word waveRAM_Address = 0xFF30;
    Byte waveRAM[16];
    Byte waveSamples[32];

public:
    // bool trigger;
    WaveChannel();
    void setMemoryMap(MemoryMap* m){ mMap = m; }
    void enableAndLoad();
    void readWaveRam();
    bool checkEnable();
    void readEnable();
    bool checkTrigger();
    bool checkLengthEnable();
    void run(Byte rateDiv);
    void step(int cycles);
    void takeSample();
    Byte getVolume();
    void readOutputLevel();
    void readSoundLengthEnable();
    void readPeriodValue();
};

class NoiseChannel
{
private:
    Word NR[5];
    Word registerAddress=0xFF1f;
    // Registers NR4x are from FF20 to FF23
    // NR[0] is not needed 

    MemoryMap* mMap;
    Byte volume;
    bool enable;
    int frequencyTimer;

    // NRx1
    // max value = 64
    Byte lengthTimer;

    // NRx2
    Byte envelopeVolume;
    Byte envelopeDirection;
    Byte envelopeSweepPace;
    Byte envelopeSweepPaceClock;


    // NRx3
    Byte clockShift;
    Byte LFSRWidth;
    Byte clockDivider;
    Word LFSR;

    Byte dividerTable[8] = {8, 16, 32, 48, 64, 80, 96, 112};

    // NRx4
    bool trigger;
    bool soundLengthEnable;

public:
    NoiseChannel();
    void setMemoryMap(MemoryMap* m){ mMap = m; }
    void enableAndLoad();
    bool checkEnable();
    bool checkTrigger();
    bool checkLengthEnable();
    void run(Byte rateDiv);
    void step(int cycles);
    void takeSample();
    Byte getVolume();
    void readSoundLengthEnable();
    void readPolynomialRegister();
};


class APU
{
private:
    int a=0;
    // SDL Audio
    // https://documentation.help/SDL/guideaudioexamples.html
    SDL_AudioSpec wanted, obtained;
    SDL_AudioDeviceID audioDeviceID;

    static Uint8 *audio_chunk;
    static Uint32 audio_len;
    static Uint8 *audio_pos;

    // memory map     
    MemoryMap* mMap;
    // Gets an audio sample every 95 clock cycles. 
    // clockSpeed/sampleRate ~ 95
    int sampleCounter;
    // This updates the frame sequencer at 512Hz.
    // Must be reset after every 8192 clock cycles.
    int frameSequencerCounter;
    int frameSequencer;
    // DIV-APU Counter
    // https://gbdev.io/pandocs/Audio_details.html#div-apu
    // increases every 512Hz
    Byte rateDIV;

    // Buffer Size
    unsigned int bufferSize = 4096;
    // Buffer
    float buffer[4096] = {0}; 

    unsigned int bufferIndex = 0; 

    int sampleRate; 
    int sampleRateTimer; 

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
    void executeAPU();
    void setMemoryMap(MemoryMap* m){ mMap = m; }
    void test(int cycles);
    void stepAPU(int cycles);
};