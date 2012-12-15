/*
 * Sound.h:
 *  A smart reference to a sound file.
 */

#ifndef LSOUND_SOUND_H_
#define LSOUND_SOUND_H_

#include <string>
#include <cstdio>
#include <lcommon/smartptr.h>

struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;

namespace lsound {

	class Sound {
	public:
		Sound();
		~Sound();
		Sound(const std::string& filename);

		void init(const std::string& filename);

		/* Clear the reference*/
		void clear();

		bool empty() const;

		void play() const;
		void loop() const;
	private:
		smartptr<Mix_Music> _music;
	};

}

#endif /* LSOUND_SOUND_H_ */
