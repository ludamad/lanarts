/*
 *GameInst.cpp:
 * Base object of the game object inheritance heirarchy
 * Life cycle of GameInst initialization:
 * ->Constructor
 * ->GameState::add_instance(obj) called
 * 		->GameInstSet::add(obj) called from add_instance, sets id
 * 		->GameInst::init(GameState) called from add_instance, does further initialization
 */

#include "GameInst.h"

#include "../serialize/SerializeBuffer.h"
#include "../gamestate/GameState.h"
#include "../display/display.h"

GameInst::~GameInst() {
}

void GameInst::step(GameState* gs) {
}

void GameInst::draw(GameState* gs) {
}

void GameInst::init(GameState* gs) {
}

void GameInst::deinit(GameState* gs) {
	id = 0;
	current_level = -1;
}

unsigned int GameInst::integrity_hash() {
	unsigned int hash = 0x9a3e;
	hash ^= (x << 16) + y;
	hash ^= this->radius * hash;
	return hash;
}

void GameInst::retain_reference() {
	reference_count++;
}

void GameInst::update_position(float newx, float newy) {
	this->x = round(newx), this->y = round(newy);
}

void GameInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	//Write the plain-old-data region
	//Dont save reference count or id
	SERIALIZE_POD_REGION(serializer, this, last_x, current_level);
	lua_variables.serialize(gs->get_luastate(), serializer);
}

void GameInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	//Read the plain-old-data region
	//Dont load reference count or id
	DESERIALIZE_POD_REGION(serializer, this, last_x, current_level);
	lua_variables.deserialize(gs->get_luastate(), serializer);
}

void GameInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(false);
}

GameInst* GameInst::clone() const {
	LANARTS_ASSERT(false);
	return NULL;
}

void GameInst::free_reference() {
	reference_count--;
	if (reference_count <= 0) {
		delete this;
	}
}

//Probably safer to force all implementations to define their own copy_to function
//void GameInst::copy_to(GameInst *inst){
//	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
//	*inst = *this;
//}

