#pragma once
#include "types.h"
#include <stdio.h>
#include <SDL.h>

enum Channel
{
	CH1 = 0,
	CH2 = 1,
	CH3 = 2,
	CH4 = 3
};

class PulseChannel
{
private:
	Channel channel;
    bool enabled;
    bool dacEnabled;

	Byte sweepPeriod;
	bool sweepNegate;
	Byte sweepShift;

	// NRx1
	Byte waveDuty;
	int lengthTimer;

	Byte envelopeInitialVolume;
	bool envelopeIncrease;
	Byte envelopePeriod;

	int frequency;

	bool soundLengthEnable;

public:
	PulseChannel(Channel channel);
	void test();
	void writeByte(Word address, Byte value);
	Byte readByte(Word address);
    bool isEnabled();
    void powerOff();
};

class WaveChannel
{
private:

	Byte waveRAM[16];
	bool dacEnabled;
	bool enabled;

	int lengthTimer;
	int maxLengthTimer;

	Byte outputLevel;

	int frequency;

	bool soundLengthEnable;

public:
	WaveChannel();
	void test();
	void writeByte(Word address, Byte value);
	Byte readByte(Word address);
	void trigger();
    bool isEnabled();
    void powerOff();
};

class NoiseChannel
{
private:
    bool enabled;
    bool dacEnabled;
	int lengthTimer;
	int maxLengthTimer;

	Byte envelopeInitialVolume;
	bool envelopeIncrease;
	Byte envelopePeriod;

	// NRx3
	Byte clockShift;
	bool LFSRWidthMode;
	Byte clockDivider;
	Word LFSR;

	Byte dividerTable[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };

	// NRx4
	// bool trigger;
	bool soundLengthEnable;

public:
	NoiseChannel();
	void test();
	void writeByte(Word address, Byte value);
	Byte readByte(Word address);
	void trigger();
    bool isEnabled();
    void powerOff();
};

class APU
{
private:
	// SDL Audio
	// https://documentation.help/SDL/guideaudioexamples.html
	SDL_AudioSpec wanted, obtained;
	SDL_AudioDeviceID audioDeviceID;

	static Uint8* audio_chunk;
	static Uint32 audio_len;
	static Uint8* audio_pos;

	// Gets an audio sample every 95 clock cycles.
	// clockSpeed/sampleRate ~ 95
	int sampleCounter;

	// This updates the frame sequencer at 512Hz.
	// Must be reset after every 8192 clock cycles.
	int frameSequencerCounter;
	int frameSequencer;

	// Buffer
	unsigned int bufferSize = 4096;
	unsigned int bufferIndex = 0;
	float buffer[4096] = { 0 };

	bool enabled;

	Byte soundPann;

	bool enableVINLeft;
	bool enableVINRight;
	Byte volumeLeft;
	Byte volumeRight;

	// Audio Channels
	PulseChannel* channel1;
	PulseChannel* channel2;
	WaveChannel* channel3;
	NoiseChannel* channel4;

public:
	APU();
	void test();
	void writeByte(Word address, Byte value);
	Byte readByte(Word address);
	void stepAPU(int cycles);
	void clearRegisters();
};