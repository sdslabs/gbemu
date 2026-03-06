#pragma once

#include <SDL.h>

#include "types.h"
#include "mmap.h"

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
	int frameSequencer;

	Byte sweepPeriod;
	bool sweepNegate;
	Byte sweepShift;
	bool sweepEnabled;
	int sweepTimer;
	int shadowFrequency;
	bool negateHasBeenUsed;

	// NRx1
	Byte waveDuty;
	int lengthTimer;
	int maxLengthTimer = 64;

	Byte envelopeInitialVolume;
	bool envelopeIncrease;
	Byte envelopePeriod;
	int envelopeTimer;
	Byte currentVolume;

	int frequency;

	bool soundLengthEnable;

	// Frequency timer for waveform generation
	int frequencyTimer;
	int waveformPosition;

public:
	PulseChannel(Channel channel);
	void writeByte(Word address, Byte value);
	Byte readByte(Word address);
	bool isEnabled();
	void powerOff();
	void run();
	void set_NRx4(Byte value);
	void setFrameSequencer(int frameSequencer);
	void trigger();
	int calculateSweep();
	void performSweep();
	void clockEnvelope();
	void step(int cycles);
};

class WaveChannel
{
private:
	Byte waveRAM[16];
	bool dacEnabled;
	bool enabled;

	int lengthTimer;
	int maxLengthTimer = 256;
	int frameSequencer;

	Byte outputLevel;

	int frequency;

	bool soundLengthEnable;

	int frequencyTimer;
	int wavePosition;
	Byte sampleBuffer;
	Byte previousSample;
	bool firstSampleWindow;

	// DMG wave RAM access tracking
	// Tracks whether the wave channel just accessed wave RAM on the last T-cycle
	bool waveFormJustRead;
	// Tracks how many cycles were pre-stepped during a wave RAM read
	int preSteppedCycles;


	int getTimerReload() const;
	void advanceWavePosition();
	// Advances the wave timer by an arbitrary number of cycles, tracking whether
	// a wave RAM access lands on the final T-cycle. This exists because the
	// emulator steps the APU in batches (once per CPU instruction), but wave RAM
	// collision detection requires knowing the channel's state at the exact
	// T-cycle of a CPU memory access (M3 = 8T into the instruction).
	void stepInternal(int cycles);

public:
	WaveChannel();
	void writeByte(Word address, Byte value);
	Byte readByte(Word address);
	void trigger();
	bool isEnabled() const;
	void powerOff();
	void set_NRx4(Byte value);
	void run();
	void setFrameSequencer(int frameSequencer);
	void step(int cycles);
};

class NoiseChannel
{
private:
	bool enabled;
	bool dacEnabled;

	int lengthTimer;
	int maxLengthTimer = 64;
	int frameSequencer;

	Byte envelopeInitialVolume;
	bool envelopeIncrease;
	Byte envelopePeriod;
	int envelopeTimer;
	Byte currentVolume;

	// NRx3
	Byte clockShift;
	bool LFSRWidthMode;
	Byte clockDivider;
	Word LFSR;

	Byte dividerTable[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };

	int frequencyTimer;

	bool soundLengthEnable;

public:
	NoiseChannel();
	void writeByte(Word address, Byte value);
	Byte readByte(Word address);
	void trigger();
	bool isEnabled();
	void powerOff();
	void set_NRx4(Byte value);
	void run();
	void setFrameSequencer(int frameSequencer);
	void clockEnvelope();
	void step(int cycles);
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

	bool enabled;

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

	// Pointer to MemoryMap
	MemoryMap* mMap;

public:
	APU();
	void setMemoryMap(MemoryMap* mMap);
	bool init();
	void writeByte(Word address, Byte value);
	Byte readByte(Word address);
	void stepAPU(int cycles);
	void clearRegisters();
	void initializeReadWriteHandlers();
	int getFrameSequencer() const { return frameSequencer; }
};