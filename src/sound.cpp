#include "sound.h"
#include "types.h"
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();

APU::APU()
{     
	SDL_zero(wanted);
	SDL_zero(obtained);
	audioDeviceID = 0;

    mMap = nullptr;

	// counter variables
	sampleCounter = 0;
	frameSequencerCounter = 0;
	frameSequencer = 0; 

    // Audio Controllers
    enableOutput = 0;
    channelEnable[4] = {false};

	frameSequencer = 0;

    soundPann = 0;
	sampleRate = 32;
	sampleRateTimer = 0;

    enableVINLeft = 0;
    enableVINRight = 0;

    volumeLeft = 0;
    volumeRight = 0;
    
    //Audio Channels
    channel1 = new PulseChannel();
	channel2 = new PulseChannel();
    channel3 = new WaveChannel();
    channel4 = new NoiseChannel();
}


bool APU::init(){
    // Initializing SDL Audio
    wanted.freq = 44100;
    wanted.format = AUDIO_F32SYS;
    wanted.channels = 2;    /* 1 = mono, 2 = stereo */
    wanted.samples = bufferSize;  
    wanted.callback = NULL;
    wanted.userdata = NULL;

	audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &wanted, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if(	audioDeviceID == 0 ){
		printf("SDL Audio not initialize! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}
	SDL_PauseAudioDevice(audioDeviceID,0);
	SDL_Delay(3);

	
	channel1->setMemoryMap(mMap);
	channel2->setMemoryMap(mMap);
	channel3->setMemoryMap(mMap);
	channel4->setMemoryMap(mMap);

	channel1->init(1);
	channel2->init(2);
	channel3->readEnable();

	return true;
}


void APU::stepAPU(int cycles){
	sampleCounter+=cycles;
	frameSequencerCounter+=cycles;

	if(frameSequencerCounter>=8192){
		//update envelope clocks and length timers
		
		channel1->run(frameSequencer);
		channel2->run(frameSequencer);
		channel3->run(frameSequencer);
		channel4->run(frameSequencer);

		frameSequencerCounter-=8192;
		frameSequencer = (frameSequencer + 1) % 8;
	}

	// increment individual frequency timers of channels
	channel1->step(cycles);
	channel2->step(cycles);
	channel3->step(cycles);
	channel4->step(cycles);

	if(sampleCounter>=95){
		//get a new sample
		sampleCounter-=95;

		float vol = 0;
		float vol1 = (float)channel1->getVolume() / 100;
		float vol2 = (float)channel2->getVolume() / 100;
		float vol3 = (float)channel3->getVolume() / 100;
		float vol4 = (float)channel4->getVolume() / 100;
		
 		vol= vol1+vol2+vol3+vol4;
		vol /= 4.0;
		// printf("vol1: %f, vol2: %f, vol3: %f, vol4: %f\n", vol1, vol2, vol3, vol4);
		// if(vol4) printf("\n\tvol4: %f\n", vol4);
		// SDL_MixAudioFormat((Uint8*)&vol, (Uint8*)&vol1, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME);
		// SDL_MixAudioFormat((Uint8*)&vol, (Uint8*)&vol2, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME);
		// SDL_MixAudioFormat((Uint8*)&vol, (Uint8*)&vol3, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME);
		buffer[bufferIndex] = vol;
		buffer[bufferIndex + 1] = vol;
		bufferIndex += 2;

		if(bufferIndex >= bufferSize){
			bufferIndex = 0;

			while(SDL_GetQueuedAudioSize(audioDeviceID) > bufferSize * sizeof(float)){
				SDL_Delay(1);
			}

			SDL_QueueAudio(audioDeviceID, buffer, bufferSize * sizeof(float) );
		}
	}
}


void APU::test(int cycles)
{
	a+=cycles;
	if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() >1){
		printf("%d\n",a);
		a=0;
		start = std::chrono::high_resolution_clock::now();
	}

}


// ------------ Pulse Channel ------------

PulseChannel::PulseChannel()
{
	regAddr = 0;

	sweepPresent = 0;

	enable = 0;
	frequencyTimer = 0;

	sweepPace = 0;
	sweepPaceClock = 0;
    sweepChange = 0;
    sweepSlope = 0;

    waveDuty = 0;
    lengthTimer = 0;
	lengthTimerClock=0;

    envelopeVolume = 0;
    envelopeDirection = 0;
    envelopeSweepPace = 0;
	envelopeSweepPaceClock = 0;
	
	periodValue = 0;
	periodValueTemp = 0; 
	periodValueClock = 0; 
    
	trigger = 0;
    soundLengthEnable = 0;

	volume = 0;
	WaveDutyCounter = 0;
	
}

bool PulseChannel::init(Byte channelNum)
{
	switch (channelNum)
	{
	// calculating address of NRx0; note NR10 esists but NR20 does not exist 
	case 1:
		regAddr = 0xFF10;
		sweepPresent = 1;  
		break;
	case 2:
		regAddr = 0xFF15;
		break;
	default:
		printf("Pulse Channel can only be initialised with 1 or 2!\n");
		return 0;
		break;
	}

	// fill the addresses of NRxyenable 
	for(Byte i = 0; i< 5; i++) NR[i] = regAddr + i;
	return 1;
}

void PulseChannel::run(Byte frameSequencer)
{
	// 256 Hz
	// sound length 
	if(soundLengthEnable && frameSequencer % 2 == 0){
		lengthTimer++;
		if(lengthTimer >= 63){
			enable = 0;
			lengthTimer = 0;
		}
	}

	// 128 Hz
	// CH1 freq sweep
	if( sweepPresent && sweepPace > 0 && frameSequencer % 4 == 0){
		if(sweepPaceClock == 0 ){
			// calculate 11 bits from (higher 3 bits) NRx4 + (lower 8 bits) NRx3
			periodValue = ( mMap->readMemory(NR[4]) & 0b00000111 ) << 8 | ( mMap->readMemory(NR[3]) & 0b11111111 );
			
			if(sweepChange == 0){
				periodValueTemp = periodValue + (periodValue / (1<< sweepSlope)) ;
			}else{
				periodValueTemp = periodValue - (periodValue / (1<< sweepSlope)) ;
			}
			
			if( periodValueTemp == 0 || periodValueTemp > 0x7FF ){
 				enable = 0;
			}else{
				Byte tempData = mMap->readMemory(NR[4]);
				tempData = (tempData & 0b11111000) | (periodValueTemp >> 8 & 0b00000111);
				mMap->writeMemory(NR[4],tempData);
				mMap->writeMemory(NR[3], (Byte)(periodValueTemp & 0x11111111) );
			}
		}
		sweepPaceClock = (sweepPaceClock + 1) % sweepPace;
	}

	// 64 Hz
	// Envelope sweep
	if(frameSequencer % 8 == 0){

		if(envelopeVolume == 0 && envelopeDirection == 0){
			enable = 0;
		}
		
		if( envelopeSweepPace != 0 ){
			if(envelopeSweepPaceClock == 0){
				if(envelopeDirection == 0){
					if( envelopeVolume > 0) envelopeVolume--;
				}
				else {
					if ( envelopeVolume < 0xF) envelopeVolume++;
				}
			}
			envelopeSweepPaceClock = (envelopeSweepPaceClock + 1) % envelopeSweepPace;
		}

	}
}

void PulseChannel::enableAndLoad()
{
	enable = 1;

	if(sweepPresent){
		sweepPace   = ( mMap->readMemory(NR[0]) & 0b01110000 ) >> 4;   // bits 6-4
		sweepChange = ( mMap->readMemory(NR[0]) & 0b00001000 ) >> 3;   // bits 3
		sweepSlope  = ( mMap->readMemory(NR[0]) & 0b00000111 ) >> 0;   // bits 2-0
	}

	waveDuty    = ( mMap->readMemory(NR[1]) & 0b11000000 ) >> 6;   // bits 7-6
	lengthTimer = ( mMap->readMemory(NR[1]) & 0b00111111 ) >> 0;   // bits 5-0

	 
	envelopeVolume     = ( mMap->readMemory(NR[2]) & 0b11110000 ) >> 4;   // bits 7-4
	envelopeDirection  = ( mMap->readMemory(NR[2]) & 0b00001000 ) >> 3;   // bits 3
	envelopeSweepPace  = ( mMap->readMemory(NR[2]) & 0b00000111 ) >> 0;   // bits 2-0

	// calculate 11 bits from (higher 3 bits) NRx4 + (lower 8 bits) NRx3
	periodValue = ( mMap->readMemory(NR[4]) & 0b00000111 ) << 8 | ( mMap->readMemory(NR[3]) & 0b11111111 ); 
	
	soundLengthEnable  = ( mMap->readMemory(NR[2]) & 0b01000000 ) >> 6;   // bit 6 

}


Byte PulseChannel::getVolume()
{
	if(enable == 0){
		if(checkTrigger()){
			enableAndLoad();			
		}
		return 0;
	}else{
		return volume;
	}
}

void PulseChannel::takeSample()
{	
	if ( enable == 1){
		// if( periodValueClock == 0){
			volume = envelopeVolume;

			if(waveDutyTab[waveDuty][WaveDutyCounter] == 0 ){
				volume = 0;
			}
			WaveDutyCounter = (WaveDutyCounter + 1) % 8;
		// }
		// TODO: Properly Implement this
		// takes sample with frequency 1048576/ (2048 - periodValue) Hz
		// This only works for max period frequency = 400000 Hz
		// To be safe we only allow 1048576/4 Hz
		// periodValueClock = (periodValueClock + 1) % ( (0x800 - periodValue) > 4 ? 0x800 - periodValue : 4 );
	}else{
		volume = 0;
	}
}

void PulseChannel::step(int cycles){
	frequencyTimer -= cycles;
	if(frequencyTimer <= 0){
		readPeriodValue();
		frequencyTimer += (2048 - periodValue);
		// if(sweepPresent) printf("\tCH1: periodValue: %d\n", periodValue);
		if(enable){
			takeSample();
		}
	}
}

bool PulseChannel::checkTrigger()
{
	return ( mMap->readMemory(NR[4]) & 0b10000000 ) >> 7; // bit 7 
}

bool PulseChannel::checkEnable()
{
	return enable; 
}

void PulseChannel::readPeriodValue(){
	periodValue = ( mMap->readMemory(NR[4]) & 0b00000111 ) << 8 | ( mMap->readMemory(NR[3]) & 0b11111111 ); 
}


// ------------ Wave Channel ------------

WaveChannel::WaveChannel(){
	for (Byte i = 0; i < 5; i++)
		NR[i] = registerAddress + i;

    index = 0;
	volume = 0;
    // NRx0
    enable = 0;

    // NRx1
    lengthTimer = 0;

    // NRx2
    outputLevel = 0;

    // Nrx3
    // needs 11 bits - 3 bits from NRX4 + 8 bits from NRx3  
    periodValue = 0; 

    // NRx4

    trigger = 0;
    soundLengthEnable = 0;

    // Wave RAM
    Byte waveRAM[16];
    Byte waveSamples[32];
}

bool WaveChannel::checkEnable(){
	return enable;
}

void WaveChannel::readWaveRam(){
	enable = 0;
	for (Word i = 0; i < 16; i++)
	{
		waveRAM[i] = mMap->readMemory(waveRAM_Address + i);
	}

	for (Word i = 0; i < 32; i++)
	{
		waveSamples[i] = ( waveRAM[ i/2 ] >> ( ( ( i + 1 ) % 2 ) * 4 ) ) & 0b1111 ;
	}
	readEnable();
}

void WaveChannel::readEnable(){
	enable = mMap->readMemory(NR[0]) >> 7;
}

bool WaveChannel::checkTrigger(){
	trigger = mMap->readMemory(NR[4]) >> 7; 
	return trigger;
}

bool WaveChannel::checkLengthEnable(){
	return soundLengthEnable;
}

void WaveChannel::enableAndLoad(){
	// printf("\n\tyo\n\n");
	enable = 1;

	lengthTimer = mMap->readMemory(NR[1]);   // bits 7-0

	outputLevel = (mMap->readMemory(NR[2]) & 0b01100000) >> 5; //bits 6-5

	// calculate 11 bits from (higher 3 bits) NRx4 + (lower 8 bits) NRx3
	periodValue = ( mMap->readMemory(NR[4]) & 0b00000111 ) << 8 | ( mMap->readMemory(NR[3]) & 0b11111111 ); 
	
	soundLengthEnable  = ( mMap->readMemory(NR[2]) & 0b01000000 ) >> 6;   // bit 6 
}

void WaveChannel::readPeriodValue(){
	periodValue = ( mMap->readMemory(NR[4]) & 0b00000111 ) << 8 | ( mMap->readMemory(NR[3]) & 0b11111111 ); 
}

void WaveChannel::run(Byte frameSequencer){
	// 256 Hz
	// sound length 
	if(soundLengthEnable && frameSequencer % 2 == 0){
		lengthTimer++;
		if(lengthTimer >= 255){
			enable = 0;
			lengthTimer = 0;
		}
	// printf("lengthTimer: %d\n", lengthTimer);
	}
}

void WaveChannel::takeSample(){
	// if(mMap->readMemory(NR[2])){
	// 	printf("%x\n", mMap->readMemory(NR[2]));
	// }

	// for (Byte i = 0; i < 5; i++)
	// {
	// 	/* code */
	// printf("NR[%d]: %x\n", i,mMap->readMemory(NR[i]));
	// }
	// printf("\n");
	
	if (enable){
		if(index < 32){
			if(outputLevel){
			// printf("Index: %d\n", index);
			// printf("waveSamples[index]: %d\n", waveSamples[index]);
			// if(outputLevel>1) printf("outputLevel: %d\n", outputLevel);
			outVolume = waveSamples[index] >> (outputLevel - 1);
			// printf("outVolume: %d\n\n", outVolume);
			// getVolume();
			}else{
				outVolume = 0; 
			}
			index += 1;
		} else {
			// printf("\n\nREAD SAMPLES\n\n");
			index = 0;
			readWaveRam();
			// printf("outputLevel: %d\n\n", outputLevel);
			readOutputLevel();
			// printf("outputLevel: %d\n\n", outputLevel);
			takeSample();
		}
	} else {
		outVolume = 0;
	}
}

Byte WaveChannel::getVolume(){
	// printf("here");
	// if(outVolume) printf("outVolume: %d\n", outVolume);

	if(enable == 0){
		if(checkTrigger()){
			enableAndLoad();			
		}
		return 0;
	}else{
		// printf("yo: %d \n", outVolume);

		return outVolume;
	}
}

void WaveChannel::readOutputLevel(){
	// printf("NR[2]: %x\n", mMap->readMemory(NR[2]));
	outputLevel = (mMap->readMemory(NR[2]) >> 5 ) & 0b011;
}

void WaveChannel::readSoundLengthEnable(){
	// printf("NR[2]: %x\n", mMap->readMemory(NR[2]));
	soundLengthEnable  = ( mMap->readMemory(NR[2]) & 0b01000000 ) >> 6;   // bit 6 
}

void WaveChannel::step(int cycles){
	frequencyTimer -= cycles;
	if(frequencyTimer <= 0){
		readPeriodValue();
		frequencyTimer += (2048 - periodValue) << 1;
		// printf("\tperiodValue: %d\n", periodValue);
		if(enable){
			takeSample();
		}
	}
	// printf("frequencyTimer: %d\n", frequencyTimer);
}


// ------------ Noise Channel ------------

NoiseChannel::NoiseChannel(){
	for (Byte i = 0; i < 5; i++)
		NR[i] = registerAddress + i;

	volume = 0;
	frequencyTimer = 0;
    // NRx0
    enable = 0;

    // NRx1
    lengthTimer = 0;

    // NRx2
    envelopeVolume = 0;
    envelopeDirection = 0;
    envelopeSweepPace = 0;
	envelopeSweepPaceClock = 0;

    // NRx3
    clockShift = 0;
    LFSRWidth = 0;
    clockDivider = 0;
	LFSR = 0x7FFF;

    trigger = 0;
    soundLengthEnable = 0;

}

bool NoiseChannel::checkEnable(){
	return enable;
}

bool NoiseChannel::checkTrigger(){
	trigger = mMap->readMemory(NR[4]) >> 7; 
	return trigger;
}

bool NoiseChannel::checkLengthEnable(){
	return soundLengthEnable;
}

void NoiseChannel::enableAndLoad(){
	// printf("\n\tyo\n\n");
	enable = 1;

	lengthTimer = mMap->readMemory(NR[1]);   // bits 7-0

	envelopeVolume     = ( mMap->readMemory(NR[2]) & 0b11110000 ) >> 4;   // bits 7-4
	envelopeDirection  = ( mMap->readMemory(NR[2]) & 0b00001000 ) >> 3;   // bits 3
	envelopeSweepPace  = ( mMap->readMemory(NR[2]) & 0b00000111 ) >> 0;   // bits 2-0

	clockShift		= ( mMap->readMemory(NR[2]) & 0b11110000 ) >> 4;   // bits 7-4
	LFSRWidth		= ( mMap->readMemory(NR[2]) & 0b00001000 ) >> 3;   // bits 3
	clockDivider	= ( mMap->readMemory(NR[2]) & 0b00000111 ) >> 0;   // bits 2-0
	LFSR 			= 0x7FFF; 
	
	soundLengthEnable  = ( mMap->readMemory(NR[2]) & 0b01000000 ) >> 6;   // bit 6 
}

void NoiseChannel::run(Byte frameSequencer){
	// printf("frameSequencer: %d\n", frameSequencer);
	// 256 Hz
	// sound length 
	readSoundLengthEnable();
	if(soundLengthEnable && frameSequencer % 2 == 0){
		// printf("here\n");
		lengthTimer++;
		if(lengthTimer >= 63){
			enable = 0;
			lengthTimer = 0;
		}
	// printf("lengthTimer: %d\n", lengthTimer);
	}

	// 64 Hz
	// Envelope sweep
	if(frameSequencer % 8 == 0){
	// printf("\there too \n");

		if(envelopeVolume == 0 && envelopeDirection == 0){
			enable = 0;
		}
		
		if( envelopeSweepPace != 0 ){
			if(envelopeSweepPaceClock == 0){
				if(envelopeDirection == 0){
					if( envelopeVolume > 0) envelopeVolume--;
				}
				else {
					if ( envelopeVolume < 0xF) envelopeVolume++;
				}
			}
			envelopeSweepPaceClock = (envelopeSweepPaceClock + 1) % envelopeSweepPace;
		}
	}
}

void NoiseChannel::takeSample(){

	if ( enable == 1){
		Word temp = LFSR;
		LFSR = LFSR & 0x7FFF;
		bool result = (LFSR & 0b01) ^ ((LFSR > 1) & 0b01);
		LFSR = (LFSR >> 1) | (result << 14);

		if (LFSRWidth == 1) {
			LFSR &= 0b0111111110111111;
			LFSR |= result << 6;
		}
		if(temp != LFSR){
		// printf("\nAYO LFSR before: %x\n", temp);
		// printf("xor_result: %x\n", result);
		// printf("LFSRWidth: %d\n", LFSRWidth);
		// printf("(temp >> 1) | (result << 14) : %x  \n", (temp >> 1) | (result << 14) );
		// temp=(temp >> 1) | (result << 14);
		// printf("temp: %x\n", temp);
		// printf("temp & !(1 << 6): %x \n", temp & 0b0111111110111111);
		// printf("LFSR after: %x\n", LFSR);
		// temp = (temp >> 1) | (result << 14);
		// temp &= !(1 << 6);
		// printf("temp: %d  ", temp );
		// temp |= result << 6;
		// printf("temp: %d  ", temp );
		// if(envelopeVolume) printf("envelope volume: %d\n", envelopeVolume);
		}
		
		if (enable && (LFSR & 1) == 0)
            volume = envelopeVolume;
        else
            volume = 0;
	}else{
		volume = 0;
	}

	// printf("envelopeVolume: %d\n\n", envelopeVolume);

	
}

Byte NoiseChannel::getVolume(){
	// printf("here");
	// if(outVolume) printf("outVolume: %d\n", outVolume);

	if(enable == 0){
		if(checkTrigger()){
			enableAndLoad();			
		}
		return 0;
	}else{
		// printf("yo: %d \n", outVolume);
		return volume;
	}
}

void NoiseChannel::readSoundLengthEnable(){
	// printf("NR[2]: %x\n", mMap->readMemory(NR[2]));
	soundLengthEnable  = ( mMap->readMemory(NR[2]) & 0b01000000 ) >> 6;   // bit 6 
}

void NoiseChannel::step(int cycles){
	frequencyTimer -=cycles;

	if(frequencyTimer <= 0){
		readPolynomialRegister();
		frequencyTimer += dividerTable[clockDivider] << clockShift;

		takeSample();
	}
}

// reads NR43 register
void NoiseChannel::readPolynomialRegister(){
	clockShift		= ( mMap->readMemory(NR[2]) & 0b11110000 ) >> 4;   // bits 7-4
	LFSRWidth		= ( mMap->readMemory(NR[2]) & 0b00001000 ) >> 3;   // bits 3
	clockDivider	= ( mMap->readMemory(NR[2]) & 0b00000111 ) >> 0;   // bits 2-0
}