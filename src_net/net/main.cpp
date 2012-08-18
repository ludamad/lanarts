#include <cstdio>
#include <SDL.h>
#include <SDL_net.h>
#include "tests/tests.h"
#include "lanarts_net.h"

int main(int argc, const char** argv) {

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	} else if (lanarts_net_init(true) < 0) {
		fprintf(stderr, "Couldn't initialize SDL_net: %s\n", SDL_GetError());
		return 1;
	}

	run_unit_tests();

	return 0;
}
