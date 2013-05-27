///*
// * lua_roomgeneration.cpp:
// *  Room generation lua bindings
// */
//
//#include "GeneratedRoom.h"
//
//static int font_get_draw_width(const Font& font, const char* text) {
//	return font.get_draw_size(text).w;
//}
//
//LuaValue lua_roommetatable(lua_State* L) {
//	LuaValue meta = luameta_new(L, "Font");
//	LuaValue methods = luameta_constants(meta);
//	LuaValue getters = luameta_getters(meta);
//
//	getters["height"].bind_function(font_height);
//
//	methods["draw"].bind_function(font_draw);
//	methods["draw_wrapped"].bind_function(font_draw_wrapped);
//	methods["draw_size"].bind_function(font_get_draw_size);
//	methods["draw_width"].bind_function(font_get_draw_width);
//
//	luameta_gc<Font>(meta);
//
//	return meta;
//}
//
//void lua_register_roomgen(lua_State* L, const LuaValue& module) {
//	using namespace ldraw;
//	luawrap::install_userdata_type<Font, lua_fontmetatable>();
//
//	module["font_load"].bind_function(font_load);
//}
