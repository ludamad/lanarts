#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include <sound/lsound.h>

/* Mix_Music actually holds the music information.  */
lsound::Sound music;
lsound::Sound music2;

void handleKey(SDL_KeyboardEvent key);
void musicDone();

int main(void) {

	SDL_Surface *screen;
	SDL_Event event;
	int done = 0;

	SDL_Init(SDL_INIT_VIDEO);
	lsound::init();

	/* We're going to be using a window onscreen to register keypresses
	 in.  We don't really care what it has in it, since we're not
	 doing graphics, so we'll just throw something up there. */
	screen = SDL_SetVideoMode(320, 240, 0, 0);

	while (!done) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				done = 1;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				handleKey(event.key);
				break;
			}
		}

		/* So we don't hog the CPU */
		SDL_Delay(50);

	}

	/* This is the cleaning up part */
	lsound::deinit();
	SDL_Quit();

}

void handleKey(SDL_KeyboardEvent key) {
	switch (key.keysym.sym) {
	case SDLK_m:
		if (key.state == SDL_PRESSED) {

			/* Here we're going to have the 'm' key toggle the music on and
			 off.  When it's on, it'll be loaded and 'music' will point to
			 something valid.  If it's off, music will be NULL. */

			if (music.empty()) {

				/* Actually loads up the music */
				printf("Loading music.ogg\n");


				music.init("music.ogg");
				music2.init("music2.ogg");
				if (music.empty()) {
					printf("Could not load music!\n");
					exit(-1);
				}

				/* This begins playing the music - the first argument is a
				 pointer to Mix_Music structure, and the second is how many
				 times you want it to loop (use -1 for infinite, and 0 to
				 have it just play once) */
				music.play();
				music2.play();

				/* We want to know when our music has stopped playing so we
				 can free it up and set 'music' back to NULL.  SDL_Mixer
				 provides us with a callback routine we can use to do
				 exactly that */
				Mix_HookMusicFinished(musicDone);

			} else {
				/* Stop the music from playing */
				Mix_HaltMusic();

				/* Unload the music from memory, since we don't need it
				 anymore */
				music.clear();
			}
			break;
		}
	}
}

/* This is the function that we told SDL_Mixer to call when the music
 was finished. In our case, we're going to simply unload the music
 as though the player wanted it stopped.  In other applications, a
 different music file might be loaded and played. */
void musicDone() {
	Mix_HaltMusic();
	music.clear();
}

