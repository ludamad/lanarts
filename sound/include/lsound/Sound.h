/*
 * Sound.h:
 *  A smart reference to a sound file.
 */

#ifndef LSOUND_SOUND_H_
#define LSOUND_SOUND_H_

#include <string>
#include <cstdio>
#include <lcommon/smartptr.h>

namespace lsound {
	class SoundBase;

	class Sound {
	public:
		Sound();
		~Sound();
		Sound(const smartptr<SoundBase>& _sound);
		explicit Sound(SoundBase* _sound);

		bool empty() const;
		void play() const;
		void loop() const;
		void clear();
	private:
		smartptr<SoundBase> _sound;
	};

}

#endif /* LSOUND_SOUND_H_ */
