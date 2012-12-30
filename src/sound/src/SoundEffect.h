/*
 * SoundEffect.h:
 *  Represents a sound effect, multiple can be played at a time.
 */

#ifndef LSOUND_SOUNDEFFECT_H_
#define LSOUND_SOUNDEFFECT_H_

#include <string>
#include <cstdio>
#include <lcommon/smartptr.h>

#include <SoundBase.h>

struct Mix_Chunk;

namespace lsound {

	class SoundEffect : public SoundBase {
	public:
		SoundEffect();
		~SoundEffect();
		SoundEffect(const std::string& filename);

		void init(const std::string& filename);

		/* Clear the reference*/
		void clear();

		bool empty() const;

		virtual void play() const;
		virtual void loop() const;
	private:
		smartptr<Mix_Chunk> _soundeffect;
	};

}


#endif /* LSOUND_SOUNDEFFECT_H_ */
