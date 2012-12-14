/*
 * lsound.h:
 *  Routines with global-effect, and inclusion of other needed headers.
 */

#ifndef LSOUND_LSOUND_H_
#define LSOUND_LSOUND_H_

#include <sound/Sound.h>

namespace lsound {
	int init(int nchannels = 8);
	void deinit();
	void stop_music();
}

#endif /* LSOUND_LSOUND_H_ */
