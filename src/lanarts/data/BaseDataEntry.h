/*
 * BaseDataEntry.h:
 *  The base of various data entries, allows for operations common to all describable entries
 */

#ifndef BASEDATAENTRY_H_
#define BASEDATAENTRY_H_

#include <string>

#include "../lanarts_defines.h"

class BaseDataEntry {
public:
	std::string name, description;
	virtual ~BaseDataEntry() {
	}
	virtual sprite_id get_sprite() = 0;
	virtual const char* entry_type() = 0;
};

#endif /* BASEDATAENTRY_H_ */
