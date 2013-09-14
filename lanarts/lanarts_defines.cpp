#include <SDL.h>

#include <lsound/lsound.h>

#include <net-lib/lanarts_net.h>

#include "lanarts_defines.h"

void lanarts_system_quit() {
	lanarts_net_quit();
	SDL_QuitSubSystem(SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO);

	lsound::stop_music();
#ifndef __LINUX__
	// Problem with WaitDone for AudioDevice callback using PulseAudio: http://pulseaudio.org/ticket/866
	// We simply do not quit the audio subsystem on linux ... pity
	lsound::deinit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
#endif
}

