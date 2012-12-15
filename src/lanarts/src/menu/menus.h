/*
 * menus.h:
 *  Defines prototypes for the menu loop functions
 */

#ifndef MENUS_H_
#define MENUS_H_

class GameState;

void lobby_menu(GameState* gs, int width, int height);
int class_menu(GameState* gs, int width, int height);
int main_menu(GameState* gs, int width, int height);

#endif /* MENUS_H_ */
