/*
 * GameEventController.h:
 *  Handles dispatch of events bound to keyboard keys
 */

struct GameEvent {
	/* Events with _N at the end require a number for disambiguation */
	enum event_t {
		USE_ITEM_N,
		AUTOTARGET_CURRENT_ACTION,
		SWITCH_TO_SPELL_N,
		SWITCH_TO_WEAPON

	};
	int event_type;
	int event_num;
};

#ifndef GAMEEVENTCONTROLLER_H_
#define GAMEEVENTCONTROLLER_H_

class GameEventController {
public:
	GameEventController();
};

#endif /* GAMEEVENTCONTROLLER_H_ */
