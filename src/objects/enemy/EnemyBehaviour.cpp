#include "EnemyBehaviour.h"

#include "../../serialize/SerializeBuffer.h"

void EnemyBehaviour::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write_vector(path);
	serializer.write_int(current_action);
	SERIALIZE_POD_REGION(serializer, this, current_node, path_steps);
}

void EnemyBehaviour::deserialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.read_vector(path);
	serializer.read_int(current_action);
	DESERIALIZE_POD_REGION(serializer, this, current_node, path_steps);

}
