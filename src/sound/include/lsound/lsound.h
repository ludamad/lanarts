/*
 * lsound.h:
 *  Main library header.
 */

#ifndef LSOUND_LSOUND_H_
#define LSOUND_LSOUND_H_

#include <string>
#include "Sound.h"

namespace lsound {
	int init(int nchannels = 8);
	void deinit();
	void stop_music();
	void stop_sounds();

	Sound load_music(const std::string& filename);
	Sound load_sound(const std::string& filename);
}

#endif /* LSOUND_LSOUND_H_ */
