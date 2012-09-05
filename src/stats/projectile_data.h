/*
 * projectile_data.h
 *  Represents spell/weapon/enemy's projectile data loaded from the yaml
 */

#ifndef PROJECTILE_DATA_H_
#define PROJECTILE_DATA_H_

#include <string>
#include <vector>

#include "../lua/LuaValue.h"

#include "../lanarts_defines.h"
#include "stats.h"

#include "items/ProjectileEntry.h"

projectile_id get_projectile_by_name(const char* name);

#endif /* PROJECTILE_DATA_H_ */
