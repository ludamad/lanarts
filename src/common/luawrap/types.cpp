/*
 * types.cpp:
 *  Install types, implemented via a template-powered generic callback dispatch
 *  system.
 */

#include "types.h"

#include "../fatal_error.h"

namespace luawrap {
namespace _private {

Callbacks::Callbacks(push_t pushcallback, get_t getcallback,
		check_t checkcallback) {
	this->pushcallback = pushcallback;
	this->getcallback = getcallback;
	this->checkcallback = checkcallback;
}

Callbacks::Callbacks() {
	this->pushcallback = NULL;
	this->getcallback = NULL;
	this->checkcallback = NULL;
}

void install_callbacks(Callbacks& cb, push_t pushcallback, get_t getcallback,
		check_t checkcallback) {
#ifndef LUAWRAP_ALLOW_REDEFINE
	if (cb.checkcallback || cb.getcallback || cb.pushcallback) {
		if (cb.checkcallback != checkcallback && cb.getcallback != getcallback
				&& cb.pushcallback != pushcallback) {
			fatal_error(
					"Attempt to redefine type that has previously been installed!\n"
							"Policy is set to exit, if you would like this to be allowed configure luawrap with LUAWRAP_ALLOW_REDEFINE");
		}
	}
#endif
	cb.checkcallback = checkcallback;
	cb.pushcallback = pushcallback;
	cb.getcallback = getcallback;
}

}
}
