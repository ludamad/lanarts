/*
 * EnemyAIState.cpp:
 *  State information that affects enemy AI decisions
 */

#include "EnemyAIState.h"


void EnemyAIPath::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write_container(nodes);
	serializer.write(start_position);
	serializer.write(path_steps);
}

void EnemyAIPath::deserialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.read_container(nodes);
	serializer.read(start_position);
	serializer.read(path_steps);
}

void EnemyAction::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write_int(current_action);
	path.serialize(gs, serializer);
}

void EnemyAction::deserialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.read_int(current_action);
	path.deserialize(gs, serializer);
}

void EnemyAIState::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write(counters);
	action.serialize(gs, serializer);
}

void EnemyAIState::deserialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.read(counters);
	action.deserialize(gs, serializer);
}
