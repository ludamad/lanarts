/*
 * enemygen.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: 100397561
 */

#include "enemygen.h"
#include "../world/GameState.h"
#include "../world/objects/EnemyInst.h"
/*
void generate_enemies(const EnemyGenSettings& es, MTwist& mt,
		GeneratedLevel& level, GameState* gs) {
	GameTiles& tiles = gs->tile_grid();
	int start_x = (tiles.tile_width() - level.width()) / 2;
	int start_y = (tiles.tile_height() - level.height()) / 2;

	for (int i = 0; i < es.nmonsters; i++) {
		int etype = mt.rand(es.min_xplevel, es.max_xplevel + 1); //mt.rand(es.min_xplevel, es.max_xplevel);
		Pos epos = generate_location_byroom(mt, level);
		int ex = (epos.x + start_x) * 32 + 16;
		int ey = (epos.y + start_y) * 32 + 16;
		gs->add_instance(new EnemyInst(&game_enemy_data[etype], ex, ey));
		level.at(epos).has_instance = true;
	}
}*/

//Generates enemy monsters
void generate_enemies(const EnemyGenSettings& rs, MTwist& mt,
		GeneratedLevel& level, GameState* gs) {
	GameTiles& tiles = gs->tile_grid();
	int start_x = (tiles.tile_width() - level.width()) / 2;
	int start_y = (tiles.tile_height() - level.height()) / 2;

	int nmons = mt.rand(rs.min_monsters, rs.max_monsters+1);

	int total_chance = 0;

	for (int i = 0; i < rs.enemy_chances.size(); i++){
		total_chance += rs.enemy_chances[i].genchance;
	}
	for (int i = 0; i < nmons; i++) {
		int monster_roll = mt.rand(total_chance);
		int monstn;
		for (monstn = 0; monstn < rs.enemy_chances.size(); monstn++){
			monster_roll -= rs.enemy_chances[monstn].genchance;
			if (monster_roll < 0)
				break;
		}

		const EnemyGenChance& ec = rs.enemy_chances[monstn];
		int etype = ec.enemytype;
		int number = 1;
		if (mt.rand(100) < ec.groupchance){
			number = mt.rand(ec.groupmin, ec.groupmax+1);
			i += number-1;
		}

		int room = mt.rand(level.rooms().size());
		Region r = level.rooms()[room].room_region;
		for (int i = 0; i < number; i++){
			Pos epos = generate_location_in_region(mt, level, r);
			level.at(epos).has_instance = true;
			int ex = (epos.x + start_x) * 32 + 16;
			int ey = (epos.y + start_y) * 32 + 16;
			gs->add_instance(new EnemyInst(etype, ex, ey));
		}
	}
}
