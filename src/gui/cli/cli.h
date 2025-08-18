#ifndef BRICK_GAME_GUI_CLI_H_
#define BRICK_GAME_GUI_CLI_H_

#include <ncurses.h>
#include <time.h>

// #include "../../brick_game/test_tetris/test_tetris.h"
#include "../../brick_game/tetris/tetris.h"
// #include "../../brick_game/tetris_liliamo/tetris.h"

#define KEY_ESCAPE 27
#define ENTER_KEY 10
#define KEY_SPACE 32
#define KEY_Q_LOWER 113

void initNcurses();
void gameCycle();
bool showState(GameInfo_t info);
UserAction_t getSignal();

#endif  // BRICK_GAME_GUI_CLI_H_
