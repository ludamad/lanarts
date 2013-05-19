#include <lcommon/unittest.h>

#include "gamestate/LuaDrawableQueue.h"

typedef LuaDrawableQueue::Iterator Iterator;

SUITE(LuaDrawableQueue_tests) {
	TEST (test_iteration) {
		TestLuaState L;
		/* Ensure clean-up order with explicit block */ {
			LuaDrawableQueue queue;
			LuaValue value(L);

			/* Add the values. The iteration order should be 2,3,0,1. */
			static const double expected_iteration[] = {2, 3, 0, 1};
			for (int i = 0; i < 4; i++) {
				lua_pushnumber(L, i);
				value.pop();
				queue.add(value, 1 - i / 2, PosF());
			}

			int i = 0;
			/* Test the resulting iteration order. */
			for (Iterator iter = queue.get_iterator(); !iter.is_done();
					iter.fetch_next()) {
				double num = iter.get_current_object().to_num();
				CHECK_EQUAL(expected_iteration[i], num);
				i++;
			}
		}

	}
}
