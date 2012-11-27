/*
 * slb.h:
 *  SLB-specific helpers
 */

#ifndef SLB_H_
#define SLB_H_

#include <SLB/Manager.hpp>

namespace SLB {
inline Manager* getOrCreateManager(lua_State* L) {
	Manager* inst = Manager::getInstance(L);
	if (!inst) {
		inst = new Manager();
		inst->registerSLB(L);
	}
	return inst;
}
}

#endif /* SLB_H_ */
