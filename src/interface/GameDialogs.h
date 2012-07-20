#ifndef GAMEDIALOGS_H_
#define GAMEDIALOGS_H_

/*Windows defines this - how very evil*/
#ifdef DialogBox
#undef DialogBox
#endif

#include <vector>

class GameNetConnection;
union SDL_Event;
class GameState;
class DialogBox;

class GameDialogs {
public:
	/*Returns whether has handled event completely or not*/
	bool handle_event(GameState* gs, SDL_Event *event);

	void step(GameState* gs);
	void draw(GameState* gs) const;

	GameDialogs();
	~GameDialogs();

private:
	/*The root dialog box, note that this may pass actions to other dialog boxes*/
	DialogBox* dialog;
};

#endif /* GAMEDIALOGS_H_ */
