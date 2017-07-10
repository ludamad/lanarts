/*
 *GameInst.cpp:
 * Base object of the game object inheritance heirarchy
 * Life cycle of GameInst initialization:
 * ->Constructor
 * ->GameState::add_instance(obj) called
 * 		->GameInstSet::add(obj) called from add_instance, sets id
 * 		->GameInst::init(GameState) called from add_instance, does further initialization
 */

#include <luawrap/luawrap.h>

#include <lcommon/SerializeBuffer.h>
#include <lcommon/luaserialize.h>

#include "data/lua_util.h"
#include "draw/draw_sprite.h"

#include "gamestate/GameState.h"

#include "GameInst.h"

#include "lua_api/lua_api.h"

GameInst::~GameInst() {
}

void GameInst::lua_lookup(lua_State* L, const char* key) {
	if (lua_variables.empty() || lua_variables.isnil()) {
		lua_pushnil(L);
		return;
	}
	lua_variables[key].push();
}

bool GameInst::try_callback(const char* callback) {
	if (lua_variables.empty() || lua_variables.isnil()) {
		return false;
	}

	lua_State* L = lua_variables.luastate();
	lua_pushstring(L, callback);

	int string_idx = lua_gettop(L);
	lua_variables.push();
	lua_pushvalue(L, string_idx); // Duplicate string
	lua_gettable(L,  string_idx + 1); // Get from lua_variables

	// Not available ? Try the type table.
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		// Push type table
		lua_pushliteral(L, "type");
		lua_rawget(L, string_idx + 1); // Get from lua_variables
		if (!lua_isnil(L, -1)) {
			int type_table_idx = lua_gettop(L);
			lua_pushvalue(L, string_idx); // Duplicate string
			lua_gettable(L, type_table_idx);
		}
	}

	bool called = false;
	if (!lua_isnil(L, -1)) {
		luawrap::push(L, this);
		lua_call(L, 1, 0);
		called = true;
        }
	lua_settop(L, string_idx - 1);
	return called;
}

void GameInst::step(GameState* gs) {
	try_callback("on_step");
    if (!lua_variables.empty() && !lua_variables.isnil()) {
        lua_State* L = gs->luastate();
        lua_variables["__objectref"].push();
        GameInst** udata = (GameInst**) lua_touserdata(L, -1);
        LANARTS_ASSERT(this == *udata);
        lua_pop(L, 1);
    }
}

void GameInst::draw(GameState* gs) {
	try_callback("on_draw");
}

void GameInst::post_draw(GameState* gs) {
	try_callback("on_post_draw");
}

void GameInst::init(GameState* gs) {
	current_floor = gs->game_world().get_current_level_id();
	try_callback("on_map_init");
}

void GameInst::deinit(GameState* gs) {
	try_callback("on_deinit");
//        if (!lua_variables.empty() && !lua_variables.isnil()) {
//            lua_pushnil(gs->luastate());
//            lua_variables["__objectref"].pop();
//            lcall(luawrap::globals(gs->luastate())["deadprotect"], lua_variables);
//        }
//	lua_variables.clear();
	current_floor = -1;
}

unsigned int GameInst::integrity_hash() {
	Pos xy = ipos();
	unsigned int hash = 0x9a3e;
	hash ^= (xy.x << 16) + xy.y;
	hash ^= int(this->radius) * hash;
	return hash;
}

void GameInst::retain_reference(GameInst* inst) {
	inst->reference_count++;
}

void GameInst::update_position(float newx, float newy) {
	this->x = newx, this->y = newy;
}

void GameInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	//Write the plain-old-data region
	//Dont save reference count or id
	SERIALIZE_POD_REGION(serializer, this, last_x, current_floor);
}

// Must be done after the normal serialization to allow for referring to the object.
void GameInst::serialize_lua(GameState* gs, SerializeBuffer& serializer) {
	LuaSerializeConfig& conf = gs->luaserialize_config();
	conf.encode(serializer, lua_variables);
	serializer.write_int(0xBADBAFE);
    effects.serialize(gs, serializer);
}

void GameInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	//Read the plain-old-data region
	//Dont load reference count or id
	DESERIALIZE_POD_REGION(serializer, this, last_x, current_floor);
	times_serialized++;
}

// Must be done after the normal serialization to allow for referring to the object.
void GameInst::deserialize_lua(GameState* gs, SerializeBuffer& serializer) {
	LuaSerializeConfig& conf = gs->luaserialize_config();
	conf.decode(serializer, lua_variables);
	int hash = serializer.read_int();
	LANARTS_ASSERT(hash == 0xBADBAFE);
	effects.deserialize(gs, serializer);
}

void GameInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(false);
}

GameInst* GameInst::clone() const {
	LANARTS_ASSERT(false);
	return NULL;
}

void GameInst::free_reference(GameInst* inst) {
	inst->reference_count--;
	if (inst->reference_count <= 0) {
		delete inst;
	}
}

//Probably safer to force all implementations to define their own copy_to function
//void GameInst::copy_to(GameInst *inst){
//	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
//	*inst = *this;
//}

GameMapState* GameInst::get_map(GameState* gs) {
	return gs->get_level(current_floor);
}

std::vector<StatusEffect> GameInst::base_status_effects(GameState* gs) {
    return {};
}
