/*
 * lsound.h:
 *  Library initialization routines.
 */
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include "lsound.h"

namespace lsound {
	int init(int nchannels) {
		/* We're going to be requesting certain things from our audio
		 device, so we set them up beforehand */
		int audio_rate = 22050;
		Uint16 audio_format = MIX_DEFAULT_FORMAT; /* 16-bit stereo */
		int audio_channels = 2;
		int audio_buffers = 512;

		SDL_Init(SDL_INIT_AUDIO);

		/* This is where we open up our audio device.  Mix_OpenAudio takes
		 as its parameters the audio format we'd /like/ to have. */
		int mixcode = Mix_OpenAudio(audio_rate, audio_format, audio_channels,
				audio_buffers);
                if (mixcode == -1) {
                        printf("Music Error: Unable to open audio: %s\n", SDL_GetError());
			return mixcode;
		}
		mixcode = Mix_AllocateChannels(nchannels);
                if( mixcode < 0 )
                {
                    printf("Music Error: Unable to allocate mixing channels: %s\n", SDL_GetError());
                    return mixcode;
                }

		return 0;
	}

	void deinit() {

		Mix_CloseAudio();
	}

	void stop_music() {
		Mix_HaltMusic();
	}
}
