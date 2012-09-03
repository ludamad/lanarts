/*
 * enemygen.cpp:
 *  Enemy spawning routines
 */

#include "../gamestate/GameState.h"

#include "../gamestate/GameTiles.h"

#include "../objects/enemy/EnemyInst.h"
#include "../util/math_util.h"

#include "enemygen.h"

//Used for !spawn command
void generate_enemy_after_level_creation(GameState* gs, enemy_id etype,
		int amount) {
	MTwist& mt = gs->rng();
	GameTiles& tiles = gs->tiles();

	for (int i = 0; i < amount; i++) {
		Pos epos;
		int tries = 0;
		do {
			int rand_x = mt.rand(tiles.tile_width());
			int rand_y = mt.rand(tiles.tile_height());
			epos = centered_multiple(Pos(rand_x, rand_y), TILE_SIZE);
		} while (gs->solid_test(NULL, epos.x, epos.y, 15));
		team_id teamid = gs->teams().default_enemy_team();
		gs->add_instance(new EnemyInst(etype, epos.x, epos.y, teamid));
	}
}

static Region& enemy_region_candidate(MTwist& mt, GeneratedLevel& level) {
	std::vector<Room>& rooms = level.rooms();
	int ind1 = mt.rand(rooms.size()), ind2 = mt.rand(rooms.size());
	Room& r1 = rooms[ind1], &r2 = rooms[ind2];
	if (r1.enemies_in_room < r2.enemies_in_room) {
		return r1.room_region;
	} else {
		return r2.room_region;
	}
}
static Pos enemy_position_candidate(MTwist& mt, GeneratedLevel& level,
		const Region& region_suggestion) {
	Pos epos;
	int tries = 0;
	do {
		if (tries++ < 20) {
			epos = generate_location_in_region(mt, level, region_suggestion);
		} else {
			epos = generate_location(mt, level);
		}
	} while (level.at(epos).near_entrance);
	return epos;
}

int generate_enemy(GeneratedLevel& level, MTwist& mt, enemy_id etype,
		const Region& r, team_id teamid, int amount) {

	int mobid = amount > 1 ? -1 : level.get_next_mob_id();
	for (int i = 0; i < amount; i++) {
		Pos epos = enemy_position_candidate(mt, level, r);
		Sqr& sqr = level.at(epos);
		sqr.has_instance = true;
		if (sqr.roomID) {
			level.get_room(sqr.roomID - 1).enemies_in_room++;
		}

		Pos world_pos = level.get_world_coordinate(epos);

		level.add_instance(
				new EnemyInst(etype, world_pos.x, world_pos.y, teamid, mobid));
	}
	return amount;
}

static int get_total_chance(const EnemyGenSettings& rs) {
	int total_chance = 0;
	for (int i = 0; i < rs.enemy_chances.size(); i++) {
		total_chance += rs.enemy_chances[i].generate_chance;
	}
	return total_chance;
}

//Generates enemy monsters
void generate_enemies(const EnemyGenSettings& rs, MTwist& mt,
		GeneratedLevel& level, GameState* gs) {

	int nmons = mt.rand(rs.num_monsters);
	int total_chance = get_total_chance(rs);

	for (int i = 0; i < rs.enemy_chances.size(); i++) {
		const EnemyGenChance& ec = rs.enemy_chances[i];
		int amnt_spawns = mt.rand(ec.guaranteed_spawns);
		if (amnt_spawns > 0) {
			generate_enemy(level, mt, ec.enemytype,
					enemy_region_candidate(mt, level),
					gs->teams().default_enemy_team(), amnt_spawns);
		}
	}

	if (total_chance == 0)
		return;
	for (int i = 0; i < nmons;) {
		int monster_roll = mt.rand(total_chance);
		int monstn;
		for (monstn = 0; monstn < rs.enemy_chances.size(); monstn++) {
			monster_roll -= rs.enemy_chances[monstn].generate_chance;
			if (monster_roll < 0)
				break;
		}

		const EnemyGenChance& ec = rs.enemy_chances[monstn];

		int amnt = 1;
		if (mt.rand(100) < ec.generate_group_chance) {
			amnt = mt.rand(ec.groupsize);
		}

		i += generate_enemy(level, mt, ec.enemytype,
				enemy_region_candidate(mt, level),
				gs->teams().default_enemy_team(), amnt);
	}
}
