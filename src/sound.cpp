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

    soundPann = 0;

    enableVINLeft = 0;
    enableVINRight = 0;

    volumeLeft = 0;
    volumeRight = 0;
    
    //Audio Channels
    channel1 = nullptr;
    channel2 = nullptr;

    channel3 = nullptr;
    channel4 = nullptr;
}


bool APU::init(){
    // Initializing SDL Audio
    wanted.freq = 44100;
    wanted.format = AUDIO_U8;
    wanted.channels = 2;    /* 1 = mono, 2 = stereo */
    wanted.samples = 1024;  
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
}



void APU::test()
{
    for (int i = 0; i < 0x20; i++)
    {
    printf("------------------------ at register FF%d: %d\n\n", i, mMap->readMemory(0xff10 + i) );
    }
    
}