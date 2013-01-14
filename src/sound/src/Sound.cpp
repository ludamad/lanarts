/*
 * Sound.cpp:
 *  A smart reference to a sound file.
 */

#include <SDL/SDL_mixer.h>
#include <Sound.h>
#include <SoundBase.h>

namespace lsound {

	Sound::Sound() {
	}

	Sound::~Sound() {
	}

	Sound::Sound(const smartptr<SoundBase>& _sound) :
			_sound(_sound) {
	}

	bool Sound::empty() const {
		return _sound.empty();
	}

	void Sound::play() const {
		return _sound->play();
	}

	void Sound::loop() const {
		return _sound->loop();
	}

	void Sound::clear() {
		_sound = smartptr<SoundBase>();
	}
}
