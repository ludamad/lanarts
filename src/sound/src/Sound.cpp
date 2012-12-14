/*
 * Sound.cpp:
 *  A smart reference to a sound file.
 */

#include <sound/Sound.h>
#include <SDL/SDL_mixer.h>

/* Controls the smart pointer deletion */
static void freemusic_callback(void* mm) {
	Mix_FreeMusic((Mix_Music*)mm);
}

namespace lsound {

	Sound::Sound() {
	}

	Sound::~Sound() {
	}

	Sound::Sound(const std::string& filename) {
		init(filename);
	}

	void Sound::init(const std::string& filename) {
		Mix_Music* mm = Mix_LoadMUS(filename.c_str());
		clear();
		if (mm) {
			_music = smartptr<Mix_Music>(mm, freemusic_callback);
		}
	}

	void Sound::play() const {
		if (_music) {
			Mix_PlayMusic(_music.get(), 0);
		}
	}
	void Sound::clear() {
		_music = smartptr<Mix_Music>();
	}

	bool Sound::empty() const {
		return !_music.is_valid();
	}

	void Sound::loop() const {
		if (_music) {
			Mix_PlayMusic(_music.get(), -1);
		}
	}
}
