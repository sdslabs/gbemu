#include "audio.h"

#include <algorithm>
#include <stdexcept>

#include "types.h"

APU::APU()
{
	SDL_zero(wanted);
	SDL_zero(obtained);
	audioDeviceID = 0;

	enabled = false;
	frameSequencer = 0;
	sampleCounter = 0;
	frameSequencerCounter = 0;
	soundPann = 0;
	enableVINLeft = false;
	enableVINRight = false;
	volumeLeft = 0;
	volumeRight = 0;

	mMap = nullptr;

	channel1 = new PulseChannel(CH1);
	channel2 = new PulseChannel(CH2);
	channel3 = new WaveChannel();
	channel4 = new NoiseChannel();
}

bool APU::init()
{
	// Initializing SDL Audio
	wanted.freq = 44100;
	wanted.format = AUDIO_F32SYS;
	wanted.channels = 2; /* 1 = mono, 2 = stereo */
	wanted.samples = bufferSize;
	wanted.callback = NULL;
	wanted.userdata = NULL;

	audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &wanted, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (audioDeviceID == 0)
	{
		printf("SDL Audio not initialize! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}
	SDL_PauseAudioDevice(audioDeviceID, 0);
	SDL_Delay(3);

	channel1->setFrameSequencer(frameSequencer);
	channel2->setFrameSequencer(frameSequencer);
	channel3->setFrameSequencer(frameSequencer);
	channel4->setFrameSequencer(frameSequencer);
	return true;
}

void APU::setMemoryMap(MemoryMap* mMap)
{
	this->mMap = mMap;
	initializeReadWriteHandlers();
}

void APU::initializeReadWriteHandlers()
{
	if (!mMap)
	{
		throw std::runtime_error("MemoryMap not set in APU");
	}

	mMap->setAudioReadHandler([this](Word address)
	    { return this->readByte(address); });
	mMap->setAudioWriteHandler([this](Word address, Byte value)
	    { this->writeByte(address, value); });
}

void APU::writeByte(Word address, Byte value)
{
	// NR52 - master control - always writable
	if (address == 0xFF26)
	{
		bool enable = (value & 0x80) >> 7;

		if (enabled && !enable)
		{
			clearRegisters();
		}
		else if (!enabled && enable)
		{
			frameSequencer = 0;
			// Do NOT reset frameSequencerCounter - it stays synchronized
			// with the system DIV timer which runs continuously.
			channel1->setFrameSequencer(0);
			channel2->setFrameSequencer(0);
			channel3->setFrameSequencer(0);
			channel4->setFrameSequencer(0);
		}

		enabled = enable;
		return;
	}

	// Wave Pattern RAM is always accessible
	if (address >= 0xFF30 && address <= 0xFF3F)
	{
		channel3->writeByte(address, value);
		return;
	}

	// DMG quirk: NRx1 length registers are writable even when APU is off
	// Only length timer bits are updated; other bits (e.g. duty) are ignored
	if (!enabled)
	{
		if (address == 0xFF11)
		{
			// Only update length, preserve duty as 0 (cleared by power off)
			channel1->writeByte(address, value & 0x3F);
		}
		else if (address == 0xFF16)
		{
			channel2->writeByte(address, value & 0x3F);
		}
		else if (address == 0xFF1B)
		{
			channel3->writeByte(address, value);
		}
		else if (address == 0xFF20)
		{
			channel4->writeByte(address, value);
		}
		return;
	}

	if (address >= 0xFF10 && address <= 0xFF14)
	{
		channel1->writeByte(address, value);
		return;
	}
	else if (address >= 0xFF16 && address <= 0xFF19)
	{
		channel2->writeByte(address, value);
		return;
	}
	else if (address >= 0xFF1A && address <= 0xFF1E)
	{
		channel3->writeByte(address, value);
		return;
	}
	else if (address >= 0xFF20 && address <= 0xFF23)
	{
		channel4->writeByte(address, value);
		return;
	}

	switch (address)
	{
	case 0xFF24:
		enableVINLeft = (value & 0x80) >> 7;
		enableVINRight = (value & 0x08) >> 3;
		volumeLeft = (value & 0x70) >> 4;
		volumeRight = (value & 0x07);
		return;
	case 0xFF25:
		soundPann = value;
		return;
	default:
		return;
	}
}

Byte APU::readByte(Word address)
{
	// Wave RAM is always readable
	if (address >= 0xFF30 && address <= 0xFF3F)
	{
		return channel3->readByte(address);
	}

	// NR52 is always readable
	if (address == 0xFF26)
	{
		Byte val = (enabled ? 0x80 : 0)
		    | (channel1->isEnabled() ? 0x01 : 0)
		    | (channel2->isEnabled() ? 0x02 : 0)
		    | (channel3->isEnabled() ? 0x04 : 0)
		    | (channel4->isEnabled() ? 0x08 : 0)
		    | 0x70;
		return val;
	}

	// Registers are readable even when APU is off (return cleared + masked values)
	if (address >= 0xFF10 && address <= 0xFF14)
	{
		return channel1->readByte(address);
	}
	else if (address >= 0xFF16 && address <= 0xFF19)
	{
		return channel2->readByte(address);
	}
	else if (address >= 0xFF1A && address <= 0xFF1E)
	{
		return channel3->readByte(address);
	}
	else if (address >= 0xFF20 && address <= 0xFF23)
	{
		return channel4->readByte(address);
	}

	switch (address)
	{
	case 0xFF24:
		return (enableVINLeft ? 0x80 : 0) | (volumeLeft << 4) | (enableVINRight ? 0x08 : 0) | volumeRight;

	case 0xFF25:
		return soundPann;

	default:
		break;
	}

	return 0xFF;
}

void APU::stepAPU(int cycles)
{
	// Frame sequencer counter always advances (tied to DIV timer)
	frameSequencerCounter += cycles;

	if (!enabled)
	{
		// Keep counter in sync with system timer even when APU is off
		while (frameSequencerCounter >= 8192)
			frameSequencerCounter -= 8192;
		return;
	}

	sampleCounter += cycles;

	// Step the wave channel frequency timer
	channel3->step(cycles);

	// Step pulse and noise channel frequency timers
	channel1->step(cycles);
	channel2->step(cycles);
	channel4->step(cycles);

	while (frameSequencerCounter >= 8192)
	{
		frameSequencerCounter -= 8192;

		// Frame sequencer steps:
		// Step 0: Length
		// Step 1: (nothing)
		// Step 2: Length + Sweep
		// Step 3: (nothing)
		// Step 4: Length
		// Step 5: (nothing)
		// Step 6: Length + Sweep
		// Step 7: Envelope

		// Run with current frame sequencer value, THEN increment
		channel1->run();
		channel2->run();
		channel3->run();
		channel4->run();

		frameSequencer = (frameSequencer + 1) % 8;

		channel1->setFrameSequencer(frameSequencer);
		channel2->setFrameSequencer(frameSequencer);
		channel3->setFrameSequencer(frameSequencer);
		channel4->setFrameSequencer(frameSequencer);
	}
}

void APU::clearRegisters()
{
	enableVINLeft = 0;
	enableVINRight = 0;
	volumeLeft = 0;
	volumeRight = 0;
	enabled = 0;
	soundPann = 0;
	channel1->powerOff();
	channel2->powerOff();
	channel3->powerOff();
	channel4->powerOff();
}

// PulseChannel

PulseChannel::PulseChannel(Channel channel)
{
	this->channel = channel;
	enabled = false;
	dacEnabled = false;
	sweepPeriod = 0;
	sweepNegate = false;
	sweepShift = 0;
	sweepEnabled = false;
	sweepTimer = 0;
	shadowFrequency = 0;
	negateHasBeenUsed = false;
	waveDuty = 0;
	lengthTimer = 0;
	envelopeInitialVolume = 0;
	envelopeIncrease = false;
	envelopePeriod = 0;
	envelopeTimer = 0;
	currentVolume = 0;
	frequency = 0;
	soundLengthEnable = false;
	frameSequencer = 0;
	frequencyTimer = 0;
	waveformPosition = 0;
}

void PulseChannel::writeByte(Word address, Byte value)
{
	switch (address)
	{
	case 0xFF10:
		// NR10 - Sweep (Channel 1 only)
		if (channel == CH1)
		{
			bool oldNegate = sweepNegate;

			sweepPeriod = (value & 0x70) >> 4;
			sweepNegate = (value & 0x08) >> 3;
			sweepShift = value & 0x07;

			// Clearing negate mode after it was used disables channel
			if (negateHasBeenUsed && oldNegate && !sweepNegate)
			{
				enabled = false;
			}
		}
		return;
	case 0xFF11:
	case 0xFF16:
		// NRx1 - Sound length/Wave pattern duty
		waveDuty = (value & 0xC0) >> 6;
		lengthTimer = maxLengthTimer - (value & 0x3F);
		return;
	case 0xFF12:
	case 0xFF17:
		// NRx2 - Volume Envelope
		dacEnabled = (value & 0xF8) != 0;
		if (!dacEnabled)
			enabled = false;

		envelopeInitialVolume = (value & 0xF0) >> 4;
		envelopeIncrease = (value & 0x08) >> 3;
		envelopePeriod = value & 0x07;
		return;
	case 0xFF13:
	case 0xFF18:
		// NRx3 - Frequency lo
		frequency = (frequency & 0x0700) | value;
		return;
	case 0xFF14:
	case 0xFF19:
	{
		// NRx4 - Frequency hi + trigger + length enable
		frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);

		// Handle length counter extra clocking on NRx4 write
		set_NRx4(value);

		if (value & 0x80)
		{
			trigger();
		}
		return;
	}
	default:
		return;
	}
}

Byte PulseChannel::readByte(Word address)
{
	switch (address)
	{
	case 0xFF10:
		// NR10
		return (sweepPeriod << 4) | (sweepNegate ? 0x08 : 0) | sweepShift | 0x80;
	case 0xFF11:
	case 0xFF16:
		// NRx1 - only duty bits readable, rest return 1
		return (waveDuty << 6) | 0x3F;
	case 0xFF12:
	case 0xFF17:
		// NRx2
		return (envelopeInitialVolume << 4) | (envelopeIncrease ? 0x08 : 0) | envelopePeriod;
	case 0xFF13:
	case 0xFF18:
		// NRx3 - write only
		return 0xFF;
	case 0xFF14:
	case 0xFF19:
		// NRx4 - only length enable bit readable
		return (soundLengthEnable ? 0x40 : 0) | 0xBF;
	default:
		return 0xFF;
	}
}

bool PulseChannel::isEnabled()
{
	return enabled && dacEnabled;
}

void PulseChannel::powerOff()
{
	enabled = false;
	dacEnabled = false;
	sweepPeriod = 0;
	sweepNegate = false;
	sweepShift = 0;
	sweepEnabled = false;
	sweepTimer = 0;
	shadowFrequency = 0;
	negateHasBeenUsed = false;
	waveDuty = 0;
	// lengthTimer is NOT cleared on DMG
	envelopeInitialVolume = 0;
	envelopeIncrease = false;
	envelopePeriod = 0;
	envelopeTimer = 0;
	currentVolume = 0;
	frequency = 0;
	soundLengthEnable = false;
	frequencyTimer = 0;
	waveformPosition = 0;
}

void PulseChannel::run()
{
	// Length counter clocks on steps 0, 2, 4, 6 (even steps)
	if (frameSequencer % 2 == 0)
	{
		if (soundLengthEnable && lengthTimer > 0)
		{
			lengthTimer--;
			if (lengthTimer == 0)
			{
				enabled = false;
			}
		}
	}

	// Sweep clocks on steps 2 and 6 (Channel 1 only)
	if (channel == CH1 && (frameSequencer == 2 || frameSequencer == 6))
	{
		if (sweepTimer > 0)
		{
			sweepTimer--;
		}

		if (sweepTimer == 0)
		{
			// Reload timer; period 0 is treated as 8
			sweepTimer = sweepPeriod ? sweepPeriod : 8;

			if (sweepEnabled && sweepPeriod != 0)
			{
				performSweep();
			}
		}
	}

	// Envelope clocks on step 7
	if (frameSequencer == 7)
	{
		clockEnvelope();
	}
}

void PulseChannel::clockEnvelope()
{
	if (envelopePeriod == 0)
		return;

	if (envelopeTimer > 0)
	{
		envelopeTimer--;
	}

	if (envelopeTimer == 0)
	{
		envelopeTimer = envelopePeriod;

		if (envelopeIncrease && currentVolume < 15)
		{
			currentVolume++;
		}
		else if (!envelopeIncrease && currentVolume > 0)
		{
			currentVolume--;
		}
	}
}

void PulseChannel::set_NRx4(Byte value)
{
	bool oldEnable = soundLengthEnable;
	bool newEnable = (value & 0x40) >> 6;
	bool trigger_bit = (value & 0x80) >> 7;

	// Extra clocking logic:
	// When the frame sequencer value is odd, we are in the "first half" of the
	// length period (right after a length clock step ran and incremented FS).
	// Enabling the length counter during this window causes an extra clock.
	bool firstHalf = (frameSequencer & 1);

	if (!oldEnable && newEnable && firstHalf)
	{
		// Extra clock when enabling length counter in first half
		if (lengthTimer > 0)
		{
			lengthTimer--;
			if (lengthTimer == 0 && !trigger_bit)
			{
				enabled = false;
			}
		}
	}

	soundLengthEnable = newEnable;

	// Trigger with length == 0: set to max
	if (trigger_bit && lengthTimer == 0)
	{
		lengthTimer = maxLengthTimer;
		// If length is being enabled in first half, extra clock
		if (newEnable && firstHalf)
		{
			lengthTimer--;
		}
	}
}

void PulseChannel::setFrameSequencer(int frameSequencer)
{
	this->frameSequencer = frameSequencer;
}

void PulseChannel::trigger()
{
	enabled = dacEnabled;

	// Envelope
	currentVolume = envelopeInitialVolume;
	envelopeTimer = envelopePeriod;

	// Frequency timer
	frequencyTimer = (2048 - frequency) * 4;

	// Sweep trigger behavior (only for Channel 1)
	if (channel == CH1)
	{
		shadowFrequency = frequency;
		sweepTimer = sweepPeriod ? sweepPeriod : 8;
		sweepEnabled = (sweepPeriod != 0) || (sweepShift != 0);
		negateHasBeenUsed = false;

		// If sweep shift is non-zero, calculate immediately and check overflow
		if (sweepShift != 0)
		{
			int newFreq = calculateSweep();
			if (newFreq > 0x7FF)
			{
				enabled = false;
			}
		}
	}
}

int PulseChannel::calculateSweep()
{
	int newFreq = shadowFrequency >> sweepShift;
	if (sweepNegate)
	{
		newFreq = shadowFrequency - newFreq;
		negateHasBeenUsed = true;
	}
	else
	{
		newFreq = shadowFrequency + newFreq;
	}
	return newFreq;
}

void PulseChannel::performSweep()
{
	int newFreq = calculateSweep();

	// Check for overflow
	if (newFreq > 0x7FF)
	{
		enabled = false;
	}
	else if (sweepShift != 0)
	{
		// Update frequency and shadow register
		shadowFrequency = newFreq;
		frequency = newFreq;

		// Calculate again and check for overflow (second overflow check)
		newFreq = calculateSweep();
		if (newFreq > 0x7FF)
		{
			enabled = false;
		}
	}
}

void PulseChannel::step(int cycles)
{
	frequencyTimer -= cycles;
	while (frequencyTimer <= 0)
	{
		frequencyTimer += (2048 - frequency) * 4;
		waveformPosition = (waveformPosition + 1) & 7;
	}
}

// WaveChannel

WaveChannel::WaveChannel()
{
	std::fill_n(waveRAM, 16, Byte { 0 });
	dacEnabled = false;
	enabled = false;
	lengthTimer = 0;
	maxLengthTimer = 256;
	outputLevel = 0;
	frequency = 0;
	soundLengthEnable = false;
	frameSequencer = 0;
	frequencyTimer = 0;
	wavePosition = 0;
	sampleBuffer = 0;
	previousSample = 0xFF;
	firstSampleWindow = true;
	waveFormJustRead = false;
	preSteppedCycles = 0;
}

void WaveChannel::writeByte(Word address, Byte value)
{
	if (address >= 0xFF30 && address <= 0xFF3F)
	{
		// Wave Pattern RAM
		if (isEnabled())
		{
			stepInternal(8);
			preSteppedCycles = 8;

			if (waveFormJustRead)
			{
				waveRAM[wavePosition / 2] = value;
			}
		}
		else
		{
			waveRAM[address - 0xFF30] = value;
		}
		return;
	}
	switch (address)
	{
	case 0xFF1A:
		// NR30 - DAC enable
		dacEnabled = (value & 0x80) >> 7;
		if (!dacEnabled)
		{
			enabled = false;
		}
		return;
	case 0xFF1B:
		// NR31 - Sound length
		lengthTimer = maxLengthTimer - value;
		return;
	case 0xFF1C:
		// NR32 - Output level
		outputLevel = (value & 0x60) >> 5;
		return;
	case 0xFF1D:
		// NR33 - Frequency lo
		frequency = (frequency & 0x0700) | value;
		return;
	case 0xFF1E:
	{
		// NR34 - Frequency hi + trigger + length enable
		frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);

		// Handle length counter extra clocking
		set_NRx4(value);

		if (value & 0x80)
		{
			trigger();
		}
		return;
	}
	default:
		return;
	}
}

Byte WaveChannel::readByte(Word address)
{
	if (address >= 0xFF30 && address <= 0xFF3F)
	{
		// Wave Pattern RAM
		if (isEnabled())
		{
			// DMG: pre-step to the read point (M3 = 8T) and check if CH3
			// just accessed wave RAM; if not, return $FF.
			stepInternal(8);
			preSteppedCycles = 8;

			if (waveFormJustRead)
			{
				return waveRAM[wavePosition / 2];
			}
			return 0xFF;
		}
		return waveRAM[address - 0xFF30];
	}
	switch (address)
	{
	case 0xFF1A:
		// NR30
		return (dacEnabled ? 0x80 : 0) | 0x7F;
	case 0xFF1B:
		// NR31 - write only
		return 0xFF;
	case 0xFF1C:
		// NR32
		return (outputLevel << 5) | 0x9F;
	case 0xFF1D:
		// NR33 - write only
		return 0xFF;
	case 0xFF1E:
		// NR34
		return (soundLengthEnable ? 0x40 : 0) | 0xBF;
	default:
		return 0xFF;
	}
}

bool WaveChannel::isEnabled() const
{
	return enabled && dacEnabled;
}

void WaveChannel::powerOff()
{
	enabled = false;
	dacEnabled = false;
	// lengthTimer is NOT cleared on DMG
	outputLevel = 0;
	frequency = 0;
	soundLengthEnable = false;
	frequencyTimer = 0;
	wavePosition = 0;
	sampleBuffer = 0;
	previousSample = 0xFF;
	firstSampleWindow = true;
	waveFormJustRead = false;
	preSteppedCycles = 0;
}

void WaveChannel::set_NRx4(Byte value)
{
	bool oldEnable = soundLengthEnable;
	bool newEnable = (value & 0x40) >> 6;
	bool trigger_bit = (value & 0x80) >> 7;

	bool firstHalf = (frameSequencer & 1);

	if (!oldEnable && newEnable && firstHalf)
	{
		if (lengthTimer > 0)
		{
			lengthTimer--;
			if (lengthTimer == 0 && !trigger_bit)
			{
				enabled = false;
			}
		}
	}

	soundLengthEnable = newEnable;

	if (trigger_bit && lengthTimer == 0)
	{
		lengthTimer = maxLengthTimer;
		if (newEnable && firstHalf)
		{
			lengthTimer--;
		}
	}
}

void WaveChannel::run()
{
	// Length counter clocks on even steps
	if (frameSequencer % 2 == 0)
	{
		if (soundLengthEnable && lengthTimer > 0)
		{
			lengthTimer--;
			if (lengthTimer == 0)
			{
				enabled = false;
			}
		}
	}
}

void WaveChannel::setFrameSequencer(int frameSequencer)
{
	this->frameSequencer = frameSequencer;
}

void WaveChannel::trigger()
{
	// DMG: If wave channel is re-triggered while it's already on,
	// the currently-accessed byte can corrupt wave RAM position 0.
	// This only happens if the trigger occurs close to the wave channel
	// reading a new byte (within certain timing windows).
	// We pre-step by 8 T-cycles (M3 offset of LDH (n),A) to reach the write point,
	// then check if the wave channel access coincides with the retrigger.
	if (isEnabled())
	{
		// Pre-step to the write point (M3 offset = 8 T-cycles)
		stepInternal(8);
		preSteppedCycles = 8;

		if (frequencyTimer <= 1)
		{
			int offset = ((wavePosition + 1) >> 1) & 0xF;
			if (offset < 4)
			{
				waveRAM[0] = waveRAM[offset];
			}
			else
			{
				int alignedPos = offset & ~0x03;
				waveRAM[0] = waveRAM[alignedPos];
				waveRAM[1] = waveRAM[alignedPos + 1];
				waveRAM[2] = waveRAM[alignedPos + 2];
				waveRAM[3] = waveRAM[alignedPos + 3];
			}
		}
	}

	enabled = dacEnabled;
	wavePosition = 0;
	// Startup delay: +6 (hardware delay) +8 (batch M3 compensation) -1 (countdown off-by-one)
	frequencyTimer = getTimerReload() + 13;
	sampleBuffer = waveRAM[0];
	previousSample = 0xFF;
	firstSampleWindow = true;
	waveFormJustRead = false;
	preSteppedCycles = 0;
}

int WaveChannel::getTimerReload() const
{
	int freq = frequency & 0x7FF;
	int period = 2048 - freq;
	if (period <= 0)
	{
		period = 1;
	}
	return period * 2;
}

void WaveChannel::advanceWavePosition()
{
	wavePosition = (wavePosition + 1) & 0x1F;
	if ((wavePosition & 1) == 0)
	{
		previousSample = sampleBuffer;
		firstSampleWindow = false;
	}
	sampleBuffer = waveRAM[wavePosition / 2];
}

void WaveChannel::stepInternal(int cycles)
{
	waveFormJustRead = false;

	int cyclesLeft = cycles;
	while (cyclesLeft > frequencyTimer)
	{
		cyclesLeft -= (frequencyTimer + 1);
		frequencyTimer = getTimerReload() - 1;
		advanceWavePosition();
		waveFormJustRead = true;
	}
	frequencyTimer -= cyclesLeft;

	// If there were remaining cycles after the last access, the access
	// didn't happen on the final T-cycle, so clear the flag.
	if (cyclesLeft > 0)
	{
		waveFormJustRead = false;
	}
}

void WaveChannel::step(int cycles)
{
	if (!isEnabled())
	{
		return;
	}

	// Account for any cycles already pre-stepped during a wave RAM read
	int remaining = cycles - preSteppedCycles;
	preSteppedCycles = 0;

	if (remaining > 0)
	{
		stepInternal(remaining);
	}
}

NoiseChannel::NoiseChannel()
{
	enabled = false;
	dacEnabled = false;
	lengthTimer = 0;
	maxLengthTimer = 64;
	clockShift = 0;
	LFSRWidthMode = false;
	clockDivider = 0;
	LFSR = 0x7FFF;
	soundLengthEnable = false;
	frameSequencer = 0;
	envelopeInitialVolume = 0;
	envelopeIncrease = false;
	envelopePeriod = 0;
	envelopeTimer = 0;
	currentVolume = 0;
	frequencyTimer = 0;
}

void NoiseChannel::writeByte(Word address, Byte value)
{
	switch (address)
	{
	case 0xFF20:
		// NR41 - Sound length
		lengthTimer = maxLengthTimer - (value & 0x3F);
		return;
	case 0xFF21:
		// NR42 - Volume Envelope
		dacEnabled = (value & 0xF8) != 0;
		if (!dacEnabled)
			enabled = false;

		envelopeInitialVolume = (value & 0xF0) >> 4;
		envelopeIncrease = (value & 0x08) >> 3;
		envelopePeriod = value & 0x07;
		return;
	case 0xFF22:
		// NR43 - Polynomial counter
		clockShift = (value & 0xF0) >> 4;
		LFSRWidthMode = (value & 0x08) >> 3;
		clockDivider = value & 0x07;
		return;
	case 0xFF23:
	{
		// NR44 - Counter/consecutive; initial
		set_NRx4(value);

		if (value & 0x80)
		{
			trigger();
		}
		return;
	}
	default:
		return;
	}
}

Byte NoiseChannel::readByte(Word address)
{
	switch (address)
	{
	case 0xFF20:
		// NR41 - write only
		return 0xFF;
	case 0xFF21:
		// NR42
		return (envelopeInitialVolume << 4) | (envelopeIncrease ? 0x08 : 0) | envelopePeriod;
	case 0xFF22:
		// NR43
		return (clockShift << 4) | (LFSRWidthMode ? 0x08 : 0) | clockDivider;
	case 0xFF23:
		// NR44
		return (soundLengthEnable ? 0x40 : 0) | 0xBF;
	default:
		return 0xFF;
	}
}

bool NoiseChannel::isEnabled()
{
	return enabled && dacEnabled;
}

void NoiseChannel::powerOff()
{
	enabled = false;
	dacEnabled = false;
	// lengthTimer is NOT cleared on DMG
	envelopeInitialVolume = 0;
	envelopeIncrease = false;
	envelopePeriod = 0;
	envelopeTimer = 0;
	currentVolume = 0;
	clockShift = 0;
	LFSRWidthMode = false;
	clockDivider = 0;
	LFSR = 0x7FFF;
	soundLengthEnable = false;
	frequencyTimer = 0;
}

void NoiseChannel::set_NRx4(Byte value)
{
	bool oldEnable = soundLengthEnable;
	bool newEnable = (value & 0x40) >> 6;
	bool trigger_bit = (value & 0x80) >> 7;

	bool firstHalf = (frameSequencer & 1);

	if (!oldEnable && newEnable && firstHalf)
	{
		if (lengthTimer > 0)
		{
			lengthTimer--;
			if (lengthTimer == 0 && !trigger_bit)
			{
				enabled = false;
			}
		}
	}

	soundLengthEnable = newEnable;

	if (trigger_bit && lengthTimer == 0)
	{
		lengthTimer = maxLengthTimer;
		if (newEnable && firstHalf)
		{
			lengthTimer--;
		}
	}
}

void NoiseChannel::run()
{
	// Length counter clocks on even steps
	if (frameSequencer % 2 == 0)
	{
		if (soundLengthEnable && lengthTimer > 0)
		{
			lengthTimer--;
			if (lengthTimer == 0)
			{
				enabled = false;
			}
		}
	}

	// Envelope clocks on step 7
	if (frameSequencer == 7)
	{
		clockEnvelope();
	}
}

void NoiseChannel::clockEnvelope()
{
	if (envelopePeriod == 0)
		return;

	if (envelopeTimer > 0)
	{
		envelopeTimer--;
	}

	if (envelopeTimer == 0)
	{
		envelopeTimer = envelopePeriod;

		if (envelopeIncrease && currentVolume < 15)
		{
			currentVolume++;
		}
		else if (!envelopeIncrease && currentVolume > 0)
		{
			currentVolume--;
		}
	}
}

void NoiseChannel::setFrameSequencer(int frameSequencer)
{
	this->frameSequencer = frameSequencer;
}

void NoiseChannel::trigger()
{
	LFSR = 0x7FFF;
	enabled = dacEnabled;

	// Envelope
	currentVolume = envelopeInitialVolume;
	envelopeTimer = envelopePeriod;

	// Frequency timer
	int divisor = dividerTable[clockDivider];
	frequencyTimer = divisor << clockShift;
}

void NoiseChannel::step(int cycles)
{
	frequencyTimer -= cycles;
	while (frequencyTimer <= 0)
	{
		int divisor = dividerTable[clockDivider];
		frequencyTimer += divisor << clockShift;

		// Clock LFSR
		int xorResult = (LFSR & 0x01) ^ ((LFSR >> 1) & 0x01);
		LFSR >>= 1;
		LFSR |= (xorResult << 14);

		if (LFSRWidthMode)
		{
			LFSR &= ~(1 << 6);
			LFSR |= (xorResult << 6);
		}
	}
}
