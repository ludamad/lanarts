/*
 * enemygen.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: 100397561
 */


#include "enemygen.h"
#include "../world/GameState.h"
#include "../world/objects/EnemyInst.h"
#include "../data/enemy_data.h"


void generate_enemies(const EnemyGenSettings& es, MTwist& mt, GeneratedLevel& level, GameState* gs){
	GameTiles& tiles = gs->tile_grid();
	int start_x = (tiles.tile_width()-level.width())/2;
	int start_y = (tiles.tile_height()-level.height())/2;

	for (int i = 0; i < es.nmonsters; i++) {
		int etype = mt.rand(0,game_enemy_n);//mt.rand(es.min_xplevel, es.max_xplevel);

		printf("Game enemies: %d gen %d\n", game_enemy_n, etype);
		Pos epos = generate_location(mt, level);
		int ex = (epos.x+start_x) * 32 + 16;
		int ey = (epos.y+start_y) * 32 + 16;
//		gs->add_instance(new EnemyInst(&game_enemy_data[etype], ex,ey));
		level.at(epos).has_instance = true;
	}
}
