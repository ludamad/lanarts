#ifndef DATAFILES_DATA_H_
#define DATAFILES_DATA_H_

#include "../util/types_util.h"//For FilenameList

/*Specifies locations of various data files*/
struct DataFiles {
	FilenameList class_files, effect_files, enemy_files, item_files,
			level_files, spell_files, sprite_files, tile_files, tileset_files,
			weapon_files;
};

#endif /* DATAFILES_DATA_H_ */
