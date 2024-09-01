#include "audio.h"
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

// Set MemoryMap pointer
void APU::setMemoryMap(MemoryMap* mMap)
{
	this->mMap = mMap;
	// initialize Handlers
	initializeReadWriteHandlers();
}

// Initializes the read-write handlers of MemoryMap
void APU::initializeReadWriteHandlers()
{
	if (!mMap)
	{
		throw std::runtime_error("MemoryMap not set in APU");
		return;
	}

<<<<<<< HEAD
	mMap->setAudioReadHandler([this](Word address)
	    { return this->readByte(address); });
	mMap->setAudioWriteHandler([this](Word address, Byte value)
	    { this->writeByte(address, value); });
=======
	mMap->setAudioReadHandler([this](Word address) { return this->readByte(address); });
	mMap->setAudioWriteHandler([this](Word address, Byte value) { this->writeByte(address, value); });
>>>>>>> bab3ce9 (Early return)
}
void APU::test()
{
	printf("APU test\n");
}

void APU::writeByte(Word address, Byte value)
{
	printf("APU Address: %X, Value: %X\n", address, value);
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
		}

		enabled = enable;
		return;
	}
	else if (address >= 0xFF30 && address <= 0xFF3F)
	{
		// Wave Pattern RAM
		channel3->writeByte(address, value);
		return;
	}

	else if (!enabled)
	{
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
	else if (address >= 0xFF30 && address <= 0xFF3F)
	{
		// Wave Pattern RAM
		return channel3->readByte(address);
	}

	Byte val = 0;
	switch (address)
	{
	case 0xFF24:
		return (enableVINLeft ? 0x80 : 0) | (volumeLeft << 4) | (enableVINRight ? 0x08 : 0) | volumeRight;

	case 0xFF25:
		return soundPann;

	case 0xFF26:
		val = (enabled ? 0x80 : 0) | (channel1->isEnabled() ? 0x01 : 0) | (channel2->isEnabled() ? 0x02 : 0) | (channel3->isEnabled() ? 0x04 : 0) | (channel4->isEnabled() ? 0x08 : 0) | 0x70;
		printf("APU Read 0xFF26: %X\n", val);
		return val;

	default:
		break;
	}

	return 0xFF;
}

void APU::stepAPU(int cycles)
{
	sampleCounter += cycles;
	frameSequencerCounter += cycles;

	if (frameSequencerCounter >= 8192)
	{
		// update envelope clocks and length timers

		channel1->run();
		channel2->run();
		channel3->run();
		channel4->run();

		frameSequencerCounter -= 8192;
		frameSequencer = (frameSequencer + 1) % 8;

		channel1->setFrameSequencer(frameSequencer);
		channel2->setFrameSequencer(frameSequencer);
		channel3->setFrameSequencer(frameSequencer);
		channel4->setFrameSequencer(frameSequencer);
	}
}

void APU::clearRegisters()
{
	printf("APU clear registers\n");
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
	enabled = 0;
	sweepPeriod = 0;
	sweepNegate = 0;
	sweepShift = 0;
	waveDuty = 0;
	lengthTimer = 0;
	envelopeInitialVolume = 0;
	envelopeIncrease = 0;
	envelopePeriod = 0;
	frequency = 0;
	soundLengthEnable = 0;
	frameSequencer = 0;
}

void PulseChannel::writeByte(Word address, Byte value)
{
	switch (address)
	{
	case 0xFF10:
		// NR10
		// Sweep
		if (channel == CH1)
		{
			sweepPeriod = (value & 0x70) >> 4;
			sweepNegate = (value & 0x08) >> 3;
			sweepShift = value & 0x07;
		}
		return;
	case 0xFF11:
	case 0xFF16:
		// NR11
		// Sound length/Wave pattern duty
		waveDuty = (value & 0xC0) >> 6;
		lengthTimer = maxLengthTimer - (value & 0x3F);
		return;
	case 0xFF12:
	case 0xFF17:
		// NR12
		// Volume Envelope
		dacEnabled = (value & 0xF8) != 0;
		enabled &= dacEnabled;

		envelopeInitialVolume = (value & 0xF0) >> 4;
		envelopeIncrease = (value & 0x08) >> 3;
		envelopePeriod = value & 0x07;
		return;
	case 0xFF13:
	case 0xFF18:
		// NR13
		// Frequency lo
		frequency = (frequency & 0x0700) | value;
		return;
	case 0xFF14:
	case 0xFF19:
		// NR14
		// Frequency hi
		frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);
		set_NRx4(value);
		if (soundLengthEnable && lengthTimer == 0)
		{
			enabled = 0;
		}
		if (value & 0x80)
		{
			trigger();
		}
		return;
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
		// NR11 NR21
		return (waveDuty << 6) | 0x3F;
	case 0xFF12:
	case 0xFF17:
		// NR12 NR22
		return (envelopeInitialVolume << 4) | (envelopeIncrease ? 0x08 : 0) | envelopePeriod;
	case 0xFF13:
	case 0xFF18:
		// NR13 NR23
		return 0xFF;
	case 0xFF14:
	case 0xFF19:
		// NR14 NR24
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
	enabled = 0;
	sweepPeriod = 0;
	sweepNegate = 0;
	sweepShift = 0;
	waveDuty = 0;
	lengthTimer = 0;
	envelopeInitialVolume = 0;
	envelopeIncrease = 0;
	envelopePeriod = 0;
	frequency = 0;
	soundLengthEnable = 0;
	frameSequencer = 0;
}

void PulseChannel::run()
{
	// length timer
	if (frameSequencer % 2 == 0)
	{
		if (soundLengthEnable && lengthTimer)
		{
			lengthTimer--;
		}
		if (soundLengthEnable && lengthTimer == 0)
		{
			enabled = 0;
		}
	}
}

void PulseChannel::set_NRx4(Byte value)
{
	bool enable = (value & 0x40) >> 6;
	bool trigger_bit = (value & 0x80) >> 7;

	if (soundLengthEnable)
	{
		if (trigger_bit && lengthTimer == 0)
		{
			if (enable && frameSequencer & 1)
			{
				lengthTimer = maxLengthTimer - 1; // clock this
			}
			else
				lengthTimer = maxLengthTimer;
		}
	}
	else if (enable)
	{
		if (frameSequencer & 1)
		{
			if (lengthTimer > 0)
				lengthTimer--; // clock this
			else if (trigger_bit && lengthTimer == 0)
				lengthTimer = maxLengthTimer - 1; // clock this
		}
	}
	else
	{
		if (trigger_bit && lengthTimer == 0)
		{
			lengthTimer = maxLengthTimer;
		}
	}

	soundLengthEnable = enable;
}

void PulseChannel::setFrameSequencer(int frameSequencer)
{
	this->frameSequencer = frameSequencer;
}

void PulseChannel::trigger()
{
	enabled = dacEnabled;
}

// WaveChannel

WaveChannel::WaveChannel()
{
	dacEnabled = 0;
	enabled = 0;
	lengthTimer = 0;
	maxLengthTimer = 256;
	outputLevel = 0;
	frequency = 0;
	soundLengthEnable = 0;
	frameSequencer = 0;
}

void WaveChannel::writeByte(Word address, Byte value)
{
	if (address >= 0xFF30 && address <= 0xFF3F)
	{
		// Wave Pattern RAM
		waveRAM[address - 0xFF30] = value;
		return;
	}
	switch (address)
	{
	case 0xFF1A:
		// NR30
		// Sound on/off
		dacEnabled = (value & 0x80) >> 7;
		enabled &= dacEnabled;
		return;
	case 0xFF1B:
		// NR31
		// Sound length
		lengthTimer = maxLengthTimer - value;
		return;
	case 0xFF1C:
		// NR32
		// Select output level
		outputLevel = (value & 0x60) >> 5;
		return;
	case 0xFF1D:
		// NR33
		// Frequency lo
		frequency = (frequency & 0x0700) | value;
		return;
	case 0xFF1E:
		// NR34
		// Frequency hi
		frequency = (frequency & 0x00FF) | ((value & 0x07) << 8);
		set_NRx4(value);
		if (soundLengthEnable && lengthTimer == 0)
		{
			enabled = 0;
		}
		if (value & 0x80)
		{
			trigger();
		}
		return;
	default:
		return;
	}
}

Byte WaveChannel::readByte(Word address)
{
	if (address >= 0xFF30 && address <= 0xFF3F)
	{
		// Wave Pattern RAM
		return waveRAM[address - 0xFF30];
	}
	switch (address)
	{
	case 0xFF1A:
		// NR30
		return (dacEnabled ? 0x80 : 0) | 0x7F;
	case 0xFF1B:
		// NR31
		return 0xFF;
	case 0xFF1C:
		// NR32
		return (outputLevel << 5) | 0x9F;
	case 0xFF1D:
		// NR33
		return 0xFF;
	case 0xFF1E:
		// NR34
		return (soundLengthEnable ? 0x40 : 0) | 0xBF;
	default:
		return 0xFF;
	}
}

bool WaveChannel::isEnabled()
{
	return enabled && dacEnabled;
}

void WaveChannel::powerOff()
{
	enabled = 0;
	dacEnabled = 0;
	lengthTimer = 0;
	outputLevel = 0;
	frequency = 0;
	soundLengthEnable = 0;
}

void WaveChannel::set_NRx4(Byte value)
{
	bool enable = (value & 0x40) >> 6;
	bool trigger_bit = (value & 0x80) >> 7;

	if (soundLengthEnable)
	{
		if (trigger_bit && lengthTimer == 0)
		{
			if (enable && frameSequencer & 1)
			{
				lengthTimer = maxLengthTimer - 1;
			}
			else
				lengthTimer = maxLengthTimer;
		}
	}
	else if (enable)
	{
		if (frameSequencer & 1)
		{
			if (lengthTimer > 0)
				lengthTimer--;
			else if (trigger_bit && lengthTimer == 0)
				lengthTimer = maxLengthTimer - 1;
		}
	}
	else
	{
		if (trigger_bit && lengthTimer == 0)
		{
			lengthTimer = maxLengthTimer;
		}
	}

	soundLengthEnable = enable;
}

void WaveChannel::run()
{
	if (frameSequencer % 2 == 0)
	{
		if (soundLengthEnable && lengthTimer)
		{
			lengthTimer--;
		}
		if (soundLengthEnable && lengthTimer == 0)
		{
			enabled = 0;
		}
	}
}

void WaveChannel::setFrameSequencer(int frameSequencer)
{
	this->frameSequencer = frameSequencer;
}

void WaveChannel::trigger()
{
	enabled = dacEnabled;
}
// Noise Channel

NoiseChannel::NoiseChannel()
{
	enabled = 0;
	lengthTimer = 0;
	maxLengthTimer = 64;
	clockShift = 0;
	LFSRWidthMode = 0;
	clockDivider = 0;
	LFSR = 0x7FFF;
	soundLengthEnable = 0;
	frameSequencer = 0;
}

void NoiseChannel::writeByte(Word address, Byte value)
{
	switch (address)
	{
	case 0xFF20:
		// NR41
		// Sound length
		lengthTimer = maxLengthTimer - (value & 0x3F);
		return;
	case 0xFF21:
		// NR42
		// Volume Envelope
		dacEnabled = (value & 0xF8) != 0;
		enabled &= dacEnabled;

		envelopeInitialVolume = (value & 0xF0) >> 4;
		envelopeIncrease = (value & 0x08) >> 3;
		envelopePeriod = value & 0x07;
		return;
	case 0xFF22:
		// NR43
		// Polynomial counter
		clockShift = (value & 0xF0) >> 4;
		LFSRWidthMode = (value & 0x08) >> 3;
		clockDivider = value & 0x07;
		return;
	case 0xFF23:
		// NR44
		// Counter/consecutive; initial
		set_NRx4(value);
		if (soundLengthEnable && lengthTimer == 0)
		{
			enabled = 0;
		}
		if (value & 0x80)
		{
			trigger();
		}
		return;
	default:
		return;
	}
}

Byte NoiseChannel::readByte(Word address)
{
	switch (address)
	{
	case 0xFF20:
		// NR41
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
	enabled = 0;
	lengthTimer = 0;
	envelopeInitialVolume = 0;
	envelopeIncrease = 0;
	envelopePeriod = 0;
	clockShift = 0;
	LFSRWidthMode = 0;
	clockDivider = 0;
	LFSR = 0x7FFF;
	soundLengthEnable = 0;
}

void NoiseChannel::set_NRx4(Byte value)
{
	bool enable = (value & 0x40) >> 6;
	bool trigger_bit = (value & 0x80) >> 7;

	if (soundLengthEnable)
	{
		if (trigger_bit && lengthTimer == 0)
		{
			if (enable && frameSequencer & 1)
			{
				lengthTimer = maxLengthTimer - 1;
			}
			else
				lengthTimer = maxLengthTimer;
		}
	}
	else if (enable)
	{
		if (frameSequencer & 1)
		{
			if (lengthTimer > 0)
				lengthTimer--;
			else if (trigger_bit && lengthTimer == 0)
				lengthTimer = maxLengthTimer - 1;
		}
	}
	else
	{
		if (trigger_bit && lengthTimer == 0)
		{
			lengthTimer = maxLengthTimer;
		}
	}

	soundLengthEnable = enable;
}

void NoiseChannel::run()
{
	if (frameSequencer % 2 == 0)
	{
		if (soundLengthEnable && lengthTimer)
		{
			lengthTimer--;
		}
		if (soundLengthEnable && lengthTimer == 0)
		{
			enabled = 0;
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
}
