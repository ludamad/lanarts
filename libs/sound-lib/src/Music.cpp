/*
 * Music.cpp:
 *  Represents a piece of music, only one piece of music can be playing at a time.
 */

#include <SDL_mixer.h>

#include "Music.h"
#include "Sound.h"
#include "lsound.h"
#include <lcommon/sdl_headless_support.h>

/* Controls the smart pointer deletion */
static void __freemusic_callback(void* mm) {
	Mix_FreeMusic((Mix_Music*)mm);
}

namespace lsound {

	Music::Music() {
	}

	Music::~Music() {
	}

	Music::Music(const std::string& filename) {
		init(filename);
	}

	void Music::init(const std::string& filename) {
		if (lsound::muted()) {
			clear();
			return;
		}
//#ifdef EMSCRIPTEN
//    Mix_Music* mm = NULL;
//#else
		Mix_Music* mm = Mix_LoadMUS(filename.c_str());
//#endif
		clear();
		if (mm) {
			_music = smartptr<Mix_Music>(mm, __freemusic_callback);
		} else {
			printf("Music Error: Could not load '%s': %s\n",
					filename.c_str(), Mix_GetError());
		}
	}

	void Music::play() const {
		if (lsound::muted()) {
			return;
		}
		if (_music) {
			if (Mix_PlayMusic(_music.get(), 0) == -1) {
				printf("Music Error: Could not play music: %s\n", Mix_GetError());
			}
		} else {
			printf("Music Error: No music loaded to play: %s\n", Mix_GetError());
		}
	}

	void Music::clear() {
		_music = smartptr<Mix_Music>();
	}

	bool Music::empty() const {
		return _music.empty();
	}

	void Music::loop() const {
		if (lsound::muted()) {
			return;
		}
		if (_music) {
			if (Mix_PlayMusic(_music.get(), -1) == -1) {
				printf("Music Error: Could not loop music: %s\n", Mix_GetError());
			}
		} else {
			printf("Music Error: No music loaded to loop: %s\n", Mix_GetError());
		}
	}

	Sound load_music(const std::string& filename) {
		return Sound(new Music(filename));
	}
}
