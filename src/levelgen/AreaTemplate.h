/*
 * AreaTemplate.h:
 *  Represents a partially defined area. Can be a standalone level template,
 *  or embedded as a small part of another level.
 *  They can be rotated randomly or not.
 */

#ifndef AREATEMPLATE_H_
#define AREATEMPLATE_H_

#include <cstring>
#include <vector>

#include "../util/mtwist.h"
#include "../lanarts_defines.h"

#include "generated_tile.h"

class AreaTemplate {
public:
	AreaTemplate();
	void initialize(int width, const char* data);
	~AreaTemplate();

private:
	std::vector<char> _data;
	int _width, _height;
};

#endif /* AREATEMPLATE_H_ */
