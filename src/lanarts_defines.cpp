#include <SDL.h>

#include <lsound/lsound.h>

#include <net-lib/lanarts_net.h>

#include "lanarts_defines.h"

void lanarts_system_quit() {
	lanarts_net_quit();
	lsound::stop_music();
	SDL_Quit();
}
