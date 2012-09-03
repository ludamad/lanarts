/*
 * BaseDataEntry.h:
 *  The base of various data entries, allows for operations common to all describable entries
 */

#ifndef BASEDATAENTRY_H_
#define BASEDATAENTRY_H_

#include <string>

class BaseDataEntry {
public:
	std::string name, description;
	virtual ~BaseDataEntry() {
	}
};

#endif /* BASEDATAENTRY_H_ */
