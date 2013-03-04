#include <lcommon/unittest.h>
#include <SDL_net.h>

int main(int argc, char** argv) {
	if (SDLNet_Init() < 0 ) {
		printf("SDL Net failed to initialize!!\n");
	}
	run_unittests();
	return 0;
}
