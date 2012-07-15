/*
 * GameChat.cpp:
 *  Game HUD component for drawing messages.
 *  These messages include things like "A monster has appeared!" and chat messages.
 */

#include <SDL.h>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include <net/packet.h>

#include "../../data/game_data.h"

#include "../../display/display.h"

#include "../../util/colour_constants.h"
#include "../../util/math_util.h"

#include "../../procedural/enemygen.h"

#include "../../lua/lua_api.h"

#include "../objects/PlayerInst.h"

#include "../GameState.h"

#include "GameChat.h"

static void print_dupe_string(const ChatMessage& cm, const font_data& font,
		const Pos& location, float alpha) {
	if (cm.exact_copies > 1)
		gl_printf(font, Colour(0, 191, 255, alpha * 255), location.x,
				location.y, " x%d", cm.exact_copies);
}

void ChatMessage::draw(const font_data& font, float alpha, int x, int y) const {
	Colour sendcol = sender_colour, msgcol = message_colour;
	sendcol.a *= alpha, msgcol.a *= alpha;
	Pos offset(0, 0);
	if (!sender.empty()) {
		offset = gl_printf(font, sendcol, x, y, "%s: ", sender.c_str());
		x += offset.x;
	}
	offset = gl_printf(font, msgcol, x, y, message.c_str());
	x += offset.x;
	print_dupe_string(*this, font, Pos(x, y), alpha);
}

bool ChatMessage::empty() const {
	return sender.empty() && message.empty();
}

void GameChat::add_message(const ChatMessage& cm) {
	bool dupe = false;

	if (!messages.empty()) {
		if (messages.back() == cm) {
			messages.back().exact_copies++;
			dupe = true;
		}
	}

	if (!dupe)
		messages.push_back(cm);

	fade_out = 1.0f;
	fade_out_rate = 0.005f;
}

void GameChat::add_message(const std::string& msg, const Colour& colour) {
	add_message(ChatMessage("", msg, Colour(), colour));
}

bool GameChat::is_typing_message() {
	return is_typing;
}

void GameChat::clear() {
	messages.clear();
}

void GameChat::draw_player_chat(GameState* gs) const {
	const font_data& font = gs->primary_font();
	const int padding = 5;
	int line_sep = font.h + 2;

	int view_w = gs->window_view().width, view_h = gs->window_view().height;
	int chat_w = view_w, chat_h = 100;
	int chat_x = 0, chat_y = 0; //h - chat_h - TILE_SIZE;
	int text_x = chat_x + padding, text_y = chat_y + padding;

	gl_draw_rectangle(chat_x, chat_y, chat_w, chat_h,
			COL_CONSOLE_BOX.with_alpha(50 * fade_out));

	bool draw_typed_message = is_typing || !typed_message.empty();

	int start_msg = 0;
	int message_space = chat_h - padding * 2
			- (draw_typed_message ? line_sep : 0);
	int msgs_in_screen = message_space / line_sep;
	if (messages.size() > msgs_in_screen) {
		start_msg = messages.size() - msgs_in_screen;
	}

	for (int i = start_msg; i < messages.size(); i++) {
		messages[i].draw(font, fade_out, text_x, text_y);
		text_y += line_sep;
	}

	if (draw_typed_message) {
		int type_y = chat_y + chat_h - padding - line_sep;
		gl_draw_line(chat_x, type_y, chat_x + chat_w, type_y,
				Colour(200, 200, 200, fade_out * 180));
		typed_message.draw(font, fade_out, text_x, type_y + padding - 1);
	}
}

static char keycode_to_char(SDLKey keycode, SDLMod keymod) {
	const char DIGIT_SYMBOLS[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*',
			'(' };
	const char MISC_SYMBOLS[][2] = { { '`', '~' }, { ',', '<' }, { '.', '>' }, {
			'/', '?' }, { ';', ':' }, { '\'', '"' }, { '[', '{' }, { ']', '}' },
			{ '\\', '|' }, { '-', '_' }, { '=', '+' } };
	bool hitcaps = (keymod & KMOD_CAPS);
	bool hitshift = (keymod & (KMOD_LSHIFT | KMOD_RSHIFT));
	if ((hitcaps != hitshift) && isalpha(keycode)) {
		return toupper(keycode);
	} else if (hitshift) {
		if (isdigit(keycode))
			return DIGIT_SYMBOLS[keycode - '0'];
		for (int i = 0; i < sizeof(MISC_SYMBOLS) / sizeof(char) / 2; i++) {
			if (keycode == MISC_SYMBOLS[i][0])
				return MISC_SYMBOLS[i][1];
		}
	}
	return keycode;
}
static bool is_typeable_keycode(SDLKey keycode) {
	return (keycode >= SDLK_SPACE && keycode <= SDLK_z);
}

static ChatMessage from_net_packet(NetPacket& p) {
	ChatMessage msg;
	p.get_str(msg.sender);
	p.get_str(msg.message);
	p.get(msg.sender_colour);
	p.get(msg.message_colour);
	return msg;
}

static void chat_to_net_packet(const ChatMessage& msg, NetPacket& p) {
	p.add(msg.message_colour);
	p.add(msg.sender_colour);
	p.add_str(msg.message);
	p.add_str(msg.sender);
}

void GameChat::step(GameState* gs) {
	std::string& msg = typed_message.message;

	if (show_chat)
		fade_out = 1.0f;
	else if (fade_out > 0.0f)
		fade_out -= fade_out_rate;

	if (repeat_steps_left > 0)
		repeat_steps_left--;
	else if (current_key != SDLK_FIRST) {
		/*Handle keys being held down*/
		if (is_typeable_keycode(current_key)) {
			msg += keycode_to_char(current_key, current_mod);
			repeat_steps_left = NEXT_REPEAT_STEP_AMNT;
		} else if (current_key == SDLK_BACKSPACE) {
			if (msg.empty()) {
				reset_typed_message();
				is_typing = false;
			} else {
				msg.resize(msg.size() - 1);
			}
			repeat_steps_left = NEXT_BACKSPACE_STEP_AMNT;
		}
	}
	NetPacket p;
	while (gs->net_connection().get_next_packet(p,
			GameNetConnection::PACKET_CHAT_MESSAGE)) {
		add_message(from_net_packet(p));
	}
}
void GameChat::draw(GameState *gs) const {
	if (fade_out > 0.0f) {
		draw_player_chat(gs);
	}
}

static const char* skip_whitespace(const char* cstr) {
	while (*cstr != '\0' && isspace(*cstr)) {
		cstr++;
	}
	return cstr;
}
static bool starts_with(const std::string& str, const char* prefix,
		const char** content) {
	int length = strlen(prefix);
	bool hasprefix = strncmp(str.c_str(), prefix, length) == 0;
	if (hasprefix) {
		*content = skip_whitespace(str.c_str() + length);
		return true;
	}
	return false;
}

bool GameChat::handle_special_commands(GameState* gs,
		const std::string& command) {
	ChatMessage printed;
	const char* content;
	PlayerInst* p = gs->local_player();

	//Spawn monster
	if (starts_with(command, "!spawn ", &content)) {
		const char* rest = content;
		int amnt = strtol(content, (char**)&rest, 10);
		if (content == rest)
			amnt = 1;
		rest = skip_whitespace(rest);

		int enemy = get_enemy_by_name(rest, false);
		if (enemy == -1) {
			printed.message = "No such monster, '" + std::string(rest) + "'!";
			printed.message_colour = Colour(255, 50, 50);
		} else {
			printed.message = std::string(rest) + " has spawned !";
			generate_enemy_after_level_creation(gs, enemy, amnt);
			printed.message_colour = Colour(50, 255, 50);
		}
		add_message(printed);
		return true;
	}

	//Set game speed
	if (starts_with(command, "!gamespeed ", &content)) {
		int gamespeed = squish(atoi(content), 1, 200);
		gs->game_settings().time_per_step = gamespeed;
		printed.message = std::string("Game speed set.");
		printed.message_colour = Colour(50, 255, 50);
		add_message(printed);
		return true;
	}

	//Gain XP
	if (starts_with(command, "!gainxp ", &content)) {
		int xp = atoi(content);
		if (xp > 0 && xp < 999999) {
			printed.message = std::string("You have gained ") + content
					+ " experience.";
			printed.message_colour = Colour(50, 255, 50);
			add_message(printed);
			p->gain_xp(gs, xp);
		} else {
			printed.message = "Invalid experience amount!";
			printed.message_colour = Colour(255, 50, 50);
			add_message(printed);
		}
		return true;
	}

	//Create item
	if (starts_with(command, "!item ", &content)) {
		const char* rest = content;
		int amnt = strtol(content, (char**)&rest, 10);
		if (content == rest)
			amnt = 1;
		rest = skip_whitespace(rest);

		int item = get_item_by_name(rest, false);
		if (item == -1) {
			printed.message = "No such item, '" + std::string(rest) + "'!";
			printed.message_colour = Colour(255, 50, 50);
		} else {
			printed.message = std::string(rest) + " put in your inventory !";
			p->stats().equipment.inventory.add(Item(item), amnt);
			printed.message_colour = Colour(50, 255, 50);
		}
		add_message(printed);
		return true;
	}

	//Kill all monsters
	if (starts_with(command, "!killall", &content)) {
		MonsterController& mc = gs->monster_controller();
		for (int i = 0; i < mc.monster_ids().size(); i++) {
			EnemyInst* inst = (EnemyInst*)gs->get_instance(mc.monster_ids()[i]);
			if (inst) {
				inst->damage(gs, 99999);
			}
		}
		printed.message = "Killed all monsters.";
		printed.message_colour = Colour(50, 255, 50);
		add_message(printed);
		return true;
	}

	lua_State* L = gs->get_luastate();
	static LuaValue script_globals;
	if (script_globals.empty()) {
		script_globals.table_initialize(L);
//		script_globals.push(L);
//		int script = lua_gettop(L);
//		lua_pushvalue(L, LUA_GLOBALSINDEX);
//		lua_setmetatable(L, script);
//		lua_pop(L, 1);
	}

	lua_push_gameinst(L, p);
	script_globals.table_pop_value(L, "player");
	lua_push_combatstats(L, p);
	script_globals.table_pop_value(L, "stats");

	//Run lua command
	if (starts_with(command, "!lua ", &content)) {
//		std::string luafunc = std::string(content);

		int prior_top = lua_gettop(L);

		luaL_loadstring(L, content);
		if (lua_isstring(L, -1)) {
			const char* val = lua_tostring(L, -1);
			add_message(val, /*iserr ? Colour(255,50,50) :*/
			Colour(120, 120, 255));
			return true;
		}

		int lfunc = lua_gettop(L);
		script_globals.push(L);
		lua_setfenv(L, lfunc);

		bool iserr = (lua_pcall(L, 0, LUA_MULTRET, 0) != 0);

		int current_top = lua_gettop(L);

		for (; prior_top < current_top; prior_top++) {
			if (lua_isstring(L, -1)) {
				const char* val = lua_tostring(L, -1);
				add_message(val,
						iserr ? Colour(255, 50, 50) : Colour(120, 120, 255));
			}
			lua_pop(L, 1);
		}

		return true;
	}
	//Run lua file
	if (starts_with(command, "!luafile ", &content)) {
		int prior_top = lua_gettop(L);

		int err_func = luaL_loadfile(L, content);
		if (err_func) {
			const char* val = lua_tostring(L, -1);
			add_message(val, Colour(120, 120, 255));
			lua_pop(L, 1);
			return true;
		}

		int lfunc = lua_gettop(L);
		script_globals.push(L);
		lua_setfenv(L, lfunc);

		bool err_call = (lua_pcall(L, 0, 0, 0) != 0);
		if (err_call) {
			const char* val = lua_tostring(L, -1);
			add_message(val, Colour(120, 120, 255));
			lua_pop(L, 1);
		}
		return true;
	}

	return false;
}

void GameChat::toggle_chat(GameState* gs) {
	if (is_typing) {
		if (!typed_message.message.empty()) {
			typed_message.sender = gs->game_settings().username;
			typed_message.sender_colour = COL_BABY_BLUE;
			if (!handle_special_commands(gs, typed_message.message)) {
				add_message(typed_message);
				NetPacket p(0, GameNetConnection::PACKET_CHAT_MESSAGE);
				ChatMessage msg = typed_message;
				msg.sender_colour = COL_MUTED_GREEN;
				msg.message_colour = COL_PALE_GREEN;
				chat_to_net_packet(msg, p);
				p.encode_header();
				gs->net_connection().broadcast_packet(p);
			}
		} else {
			show_chat = false;
			fade_out_rate = 0.1f;
		}
		reset_typed_message();
		is_typing = false;
	} else {
		if (!show_chat)
			show_chat = true;
		else if (show_chat)
			is_typing = true;
	}
}
/*Returns whether has handled event completely or not*/
bool GameChat::handle_event(GameState* gs, SDL_Event *event) {
	int view_w = gs->window_view().width, view_h = gs->window_view().height;
	int chat_w = view_w, chat_h = 100;
	int chat_x = 0, chat_y = 0; //h - chat_h - TILE_SIZE;

	SDLKey keycode = event->key.keysym.sym;
	SDLMod keymod = event->key.keysym.mod;
	current_mod = keymod;
	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN: {
//		if (show_chat && event->button.button == SDL_BUTTON_LEFT
//				&& gs->mouse_x() < chat_w && gs->mouse_y() < chat_h) {
//			toggle_chat(gs);
//			return true;
//		}
		break;
	}
	case SDL_KEYUP: {
		if (current_key == keycode)
			current_key = SDLK_FIRST;
		/*Let GameState handle this as well*/
		break;
	}
	case SDL_KEYDOWN: {
		if (keycode == SDLK_RETURN) {
			toggle_chat(gs);
			return true;
		}
		if (is_typing) {
			std::string& msg = typed_message.message;
			if (is_typeable_keycode(keycode)) {
				msg += keycode_to_char(keycode, keymod);
				if (current_key != keycode) {
					current_key = keycode;
					repeat_steps_left = INITIAL_REPEAT_STEP_AMNT;
				}
				return true;
			}
			if (keycode == SDLK_BACKSPACE) {
				if (msg.empty()) {
					reset_typed_message();
					is_typing = false;
				} else {
					msg.resize(msg.size() - 1);
				}
				if (current_key != keycode) {
					current_key = keycode;
					repeat_steps_left = INITIAL_REPEAT_STEP_AMNT;
				}
				return true;
			}
			if (keycode == SDLK_LCTRL || keycode == SDLK_RCTRL) {
				is_typing = false;
				return true;
			}
			if (keycode == SDLK_DELETE) {
				reset_typed_message();
				is_typing = false;
				return true;
			}
		}
		break;
	}
	}
	return false;
}

void GameChat::reset_typed_message() {
	typed_message.sender.clear();
	typed_message.message.clear();
}

GameChat::GameChat() :
		typed_message(std::string(), std::string()) {
	current_key = SDLK_UNKNOWN;
	current_mod = KMOD_NONE;
	reset_typed_message();
	show_chat = true;
	fade_out = 1.0f;
	fade_out_rate = 0.05f;
	is_typing = false;
	repeat_steps_left = 0;
}

void ChatMessage::packet_add(NetPacket& packet) {
	packet.add_str(sender);
	packet.add_str(message);
	packet.add(sender_colour);
	packet.add(message_colour);
}
void ChatMessage::packet_get(NetPacket& packet) {
	packet.get_str(sender);
	packet.get_str(message);
	packet.get(sender_colour);
	packet.get(message_colour);
}

