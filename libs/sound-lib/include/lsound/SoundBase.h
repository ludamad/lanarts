/*
 * SoundBase.h:
 *  Represents a playable sound, either a sound effect or music.
 */

#ifndef LSOUND_SOUNDBASE_H_
#define LSOUND_SOUNDBASE_H_

namespace lsound {

	class SoundBase {
	public:
		SoundBase();
		virtual ~SoundBase();
		virtual void play() const = 0;
		virtual void loop() const = 0;
	};
}
#endif /* LSOUND_SOUNDBASE_H_*/
