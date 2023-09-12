#include "sound.h"
#include "types.h"


APU::APU()
{     
    mMap = nullptr;
	
	SDL_zero(wanted);
	SDL_zero(obtained);
	audioDeviceID = 0;
    // Audio Controllers
    enableOutput = 0;
    channelEnable[4] = {false};

	rateDIV = 0;

    soundPann = 0;

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
    wanted.samples = 4096;  
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
	
	channel1->init(1);
	channel2->init(2);

}



void APU::test()
{
    for (int i = 0; i < 0x20; i++)
    {
    printf("------------------------ at register FF%d: %d\n\n", i, mMap->readMemory(0xff10 + i) );
    }
    
}

void APU::executeAPU()
{	
	if(channel1->checkEnable()){
		channel1->takeSample();
	}
	// printf("volume from channel 1: %d\n", channel1->getVolume());
	
	if(((mMap->getRegDIV() & 0b1000) >> 3 ) == 1){
		rateDIV = (rateDIV + 1) % 8;

		channel1->run(rateDIV);
	}

	
}



// ------------ Pulse Channel ------------

PulseChannel::PulseChannel()
{
	regAddr = 0;

	sweepPresent = 0;

	enable = 0;

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

	// fill the addresses of NRxy
	for(Byte i = 0; i< 5; i++) NR[i] = regAddr + i;
	
}

void PulseChannel::run(Byte rateDIV)
{
	// 256 Hz
	// sound length 
	if(soundLengthEnable && rateDIV % 2 == 0){
		if(lengthTimer >= 64){
			enable = 0;
			lengthTimer = 0;
		}
		lengthTimer++;
	}

	// 128 Hz
	// CH1 freq sweep
	if( sweepPresent && sweepPace > 0 && rateDIV % 4 == 0){
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
	if(rateDIV % 8 == 0){

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
		if( periodValueClock == 0){
			volume = envelopeVolume;

			if(waveDutyTab[waveDuty][WaveDutyCounter] == 0 ){
				volume = 0;
			}
			WaveDutyCounter = (WaveDutyCounter + 1) % 8;
		}
		// TODO: Properly Implement this
		// takes sample with frequency 1048576/ (2048 - periodValue) Hz
		// This only works for max period frequency = 400000 Hz
		// To be safe we only allow 1048576/4 Hz
		periodValueClock = (periodValueClock + 1) % ( (0x800 - periodValue) > 4 ? 0x800 - periodValue : 4 );
	}else{
		volume = 0;
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
