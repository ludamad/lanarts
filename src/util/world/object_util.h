/*
 * object_util.h:
 *  Useful object queries and helper functions
 */

#ifndef OBJECT_UTIL_H_
#define OBJECT_UTIL_H_

class GameState;
class GameInst;

class ItemInst;
class EnemyInst;
class PlayerInst;

PlayerInst* player_cast(GameInst* inst);
EnemyInst* enemy_cast(GameInst* inst);
EnemyInst* item_cast(GameInst* inst);

#endif /* OBJECT_UTIL_H_ */
