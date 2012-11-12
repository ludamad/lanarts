/*
 * slb_mutabletable.h:
 *  A table with __newindex operations allowed.
 */

#ifndef SLB_MUTABLETABLE_H_
#define SLB_MUTABLETABLE_H_

#include <SLB/Table.hpp>

namespace SLB {

class MutableTable: public Table {
public:
	MutableTable(bool mutability_flag = true) :
			mutability_flag(mutability_flag) {
	}
	void setMutability(bool mutability) {
		mutability_flag = mutability;
	}
protected:
	virtual int __newindex(lua_State *L) {
		if (!mutability_flag) {
			return Table::__newindex(L);
		}
		setCache(L);
		return 0;
	}
private:
	bool mutability_flag;
};

}

#endif /* SLB_MUTABLETABLE_H_ */
