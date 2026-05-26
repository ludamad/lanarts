/*
 * lsound.h:
 *  Library initialization routines.
 */
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include "lsound.h"
#include <lcommon/sdl_headless_support.h>

namespace lsound {
	static int volume_to_mix(double volume) {
		if (volume < 0.0) {
			volume = 0.0;
		} else if (volume > 1.0) {
			volume = 1.0;
		}
		return int(volume * double(MIX_MAX_VOLUME) + 0.5);
	}

	int init(int nchannels) {
		/* We're going to be requesting certain things from our audio
		 device, so we set them up beforehand */
		int audio_rate = 22050;
		Uint16 audio_format = MIX_DEFAULT_FORMAT; /* 16-bit stereo */
		int audio_channels = 2;
		int audio_buffers = 512;

		if (SDL_Init(SDL_INIT_AUDIO) < 0) {
			printf("Music Error: Unable to initialize audio: %s\n", SDL_GetError());
			return -1;
		}

		int mix_init = Mix_Init(MIX_INIT_OGG);
		if ((mix_init & MIX_INIT_OGG) != MIX_INIT_OGG) {
			printf("Music Error: Unable to initialize OGG support: %s\n", Mix_GetError());
		}

		/* This is where we open up our audio device.  Mix_OpenAudio takes
		 as its parameters the audio format we'd /like/ to have. */
		int mixcode = Mix_OpenAudio(audio_rate, audio_format, audio_channels,
				audio_buffers);
		if (mixcode == -1) {
			printf("Music Error: Unable to open audio: %s\n", Mix_GetError());
			return mixcode;
		}
		mixcode = Mix_AllocateChannels(nchannels);
		if (mixcode < 0) {
			printf("Music Error: Unable to allocate mixing channels: %s\n",
					Mix_GetError());
			return mixcode;
		}

		return 0;
	}

	void deinit() {

		Mix_CloseAudio();
		Mix_Quit();
	}

	void stop_music() {
		Mix_HaltMusic();
	}

	void set_volume(double volume) {
		int mix_volume = volume_to_mix(volume);
		Mix_Volume(-1, mix_volume);
		Mix_VolumeMusic(mix_volume);
	}

	double get_volume() {
		return Mix_VolumeMusic(-1) / double(MIX_MAX_VOLUME);
	}
}
