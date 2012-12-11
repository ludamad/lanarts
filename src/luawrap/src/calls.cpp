/*
 * calls.cpp:
 *  Call lua functions by passing appropriate C++ types.
 */

#include <luawrap/luawraperror.h>

namespace luawrap {
	namespace _private {
		// Throws/aborts if false
		void luacall_return_check(bool check_result) {
			if (!check_result) {
				luawrap::error("Return value could not be converted when calling luawrap::call, exitting...\n");
			}
		}
	}
}

