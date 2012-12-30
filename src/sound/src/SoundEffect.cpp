/*
 * SoundEffect.cpp:
 *  Represents a sound effect, multiple can be played at a time.
 */

#include <SDL/SDL_mixer.h>

#include "SoundEffect.h"
#include "Sound.h"

/* Controls the smart pointer deletion */
static void __freesound_callback(void* mm) {
	Mix_FreeChunk((Mix_Chunk*)mm);
}

namespace lsound {

	SoundEffect::SoundEffect() {
	}

	SoundEffect::~SoundEffect() {
	}

	SoundEffect::SoundEffect(const std::string& filename) {
		init(filename);
	}

	void SoundEffect::init(const std::string& filename) {
		Mix_Chunk* snd = Mix_LoadWAV(filename.c_str());
		clear();
		if (snd) {
			_soundeffect = smartptr<Mix_Chunk>(snd, __freesound_callback);
		}
	}

	void SoundEffect::play() const {
		if (_soundeffect) {
			Mix_PlayChannel(-1, _soundeffect.get(), 0);
		}
	}
	void SoundEffect::clear() {
		_soundeffect = smartptr<Mix_Chunk>();
	}

	bool SoundEffect::empty() const {
		return !_soundeffect.is_valid();
	}

	void SoundEffect::loop() const {
		if (_soundeffect) {
			Mix_PlayChannel(-1, _soundeffect.get(), -1);
		}
	}

	Sound load_sound(const std::string& filename) {
		return Sound(new SoundEffect(filename));
	}
}
