/*
 * skills.h:
 *  Holds various skill statistics, such as proficiency with ranged weapons, etc
 */

#ifndef SKILLS_H_
#define SKILLS_H_

#include <vector>

struct lua_State;
struct CombatStats;
struct EffectiveStats;

struct Skill {
	int xp, xpneeded, xplevel;
	Skill(int xp, int xpneeded, int xplevel) :
			xp(xp), xpneeded(xpneeded), xplevel(xplevel) {
	}
};

struct SkillStats {
	/* Indexed by skill id */
	std::vector<Skill> skills;

	void process(lua_State* L, const CombatStats& basestats, EffectiveStats& effective);
};


#endif /* SKILLS_H_ */
