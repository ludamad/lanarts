#ifndef DATAFILENAMES_H_
#define DATAFILENAMESH_

#include "FilenameList.h"//For FilenameList
/*Specifies locations of various data files*/
struct DataFiles {
	FilenameList class_files, effect_files, enemy_files, item_files,
			itemgenlist_files, level_files, level_template_files, equipment_files, projectile_files,
			spell_files, sprite_files, tile_files, tileset_files, weapon_files;
};

#endif /* DATAFILENAMES_H_ */
