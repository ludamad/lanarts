/*
 * callback_util.h
 *  Helper functions for callback functionality
 */

#ifndef CALLBACK_UTIL_H_
#define CALLBACK_UTIL_H_

#include <cstdlib>

class GameState;
class GameInst;

typedef void (*object_callbackf)(GameState* gs, GameInst* obj, void* data);

struct ObjCallback {
	ObjCallback(object_callbackf callback = NULL, void* data = NULL,
			bool deallocate = false) :
			callback(callback), data(data), deallocate(deallocate) {
	}
	~ObjCallback() {
		if (deallocate)
			free(data);
	}
	void call(GameState* gs, GameInst* obj){
		if (callback) {
			callback(gs, obj, data);
		}
	}
	object_callbackf callback;
	void* data;
	bool deallocate;
};



#endif /* CALLBACK_UTIL_H_ */
