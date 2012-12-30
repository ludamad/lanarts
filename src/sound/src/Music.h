/*
 * Music.h:
 *  Represents a piece of music, only one piece of music can be playing at a time.
 */

#ifndef LSOUND_MUSIC_H_
#define LSOUND_MUSIC_H_

#include <string>
#include <cstdio>
#include <lcommon/smartptr.h>

#include <SoundBase.h>

struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;

namespace lsound {

	class Music : public SoundBase {
	public:
		Music();
		~Music();
		Music(const std::string& filename);

		void init(const std::string& filename);

		/* Clear the reference*/
		void clear();

		bool empty() const;

		virtual void play() const;
		virtual void loop() const;
	private:
		smartptr<Mix_Music> _music;
	};

}


#endif /* LSOUND_MUSIC_H_ */
