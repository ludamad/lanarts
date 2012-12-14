/*
 * lsound.h:
 *  Library initialization routines.
 */
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include <sound/lsound.h>

namespace lsound {
	int init(int nchannels) {
		/* We're going to be requesting certain things from our audio
		 device, so we set them up beforehand */
		int audio_rate = 22050;
		Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
		int audio_channels = 2;
		int audio_buffers = 4096;

		SDL_Init(SDL_INIT_AUDIO);

		/* This is where we open up our audio device.  Mix_OpenAudio takes
		 as its parameters the audio format we'd /like/ to have. */
		int mixcode = Mix_OpenAudio(audio_rate, audio_format, audio_channels,
				audio_buffers);
		if (mixcode) {
			return mixcode;
		}

		Mix_AllocateChannels(nchannels);

		return 0;
	}

	void deinit() {
		Mix_CloseAudio();
	}

	void stop_music() {
		Mix_HaltMusic();
	}
}
