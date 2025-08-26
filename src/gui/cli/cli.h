#ifndef BRICK_GAME_GUI_CLI_H_
#define BRICK_GAME_GUI_CLI_H_

#include <ncurses.h>

// for debugging
// #include "../../brick_game/tetris/tetris.h"

#define ENTER_KEY 10
#define KEY_SPACE 32
#define KEY_F_LOWER 102
#define KEY_Q_LOWER 113
// #define KEY_W_LOWER 119
#define KEY_A_LOWER 97
// #define KEY_S_LOWER 115
#define KEY_D_LOWER 100

// for general case
#include "../../brick_game/brick_game.h"

void initNcurses();
void gameLoop();
bool showState(GameInfo_t info);
bool getAction();

#ifdef DEBUG
void debugWhichState(TetrisState_t *ptr_state, char *buffer);
#endif  // DEBUG

#endif  // BRICK_GAME_GUI_CLI_H_
