/*
 * skill_data.h
 *  Represents a skill and its effect on effective stats
 */

#ifndef SKILL_DATA_H_
#define SKILL_DATA_H_

#include <string>
#include <common/lua/LuaValue.h>

struct SkillEntry {
	std::string name;
	LuaValue statmod;
};

#endif /* SKILL_DATA_H_ */
