/*
 * collision_util.h
 *  Defines functions that are often used in conjunction with collision detection
 */

#ifndef COLLISION_UTIL_H_
#define COLLISION_UTIL_H_

struct GameInst;

bool item_colfilter(GameInst* self, GameInst* other);
bool enemy_colfilter(GameInst* self, GameInst* other);
bool player_colfilter(GameInst* self, GameInst* other);

#endif /* COLLISION_UTIL_H_ */
