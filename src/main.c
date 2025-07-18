#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define WIN_INIT()        \
  {                       \
    initscr();            \
    cbreak();             \
    timeout(100);         \
    keypad(stdscr, true); \
    noecho();             \
    curs_set(0);          \
  }
#define KEY_ESCAPE 27

#define ROWS 20
#define COLS 10
#define FIG_ROWS 4
#define FIG_COLS 4

typedef enum {
  Start,
  Pause,
  Terminate,
  Left,
  Right,
  Up,
  Down,
  Action
} UserAction_t;

typedef struct {
  int **field;
  int **next;
  int score;
  int high_score;
  int level;
  int speed;
  int pause;
} GameInfo_t;

typedef struct {
  struct {
    int *row[ROWS];
    int cell[ROWS][COLS];
  } field;
  struct {
    int *row[FIG_ROWS];
    int cell[FIG_ROWS][FIG_COLS];
  } next;
  UserAction_t action;
  bool hold;
} TetrisInfo_t;

void userInput(UserAction_t action, bool hold);
TetrisInfo_t *getTetrisInfo();
GameInfo_t *getGameInfo();
void game();
void userInput(UserAction_t action, bool hold);
GameInfo_t updateCurrentState();
void showState(GameInfo_t info);
void gameCycle();

TetrisInfo_t *getTetrisInfo() {
  static TetrisInfo_t *ptr_tetris_info = NULL;
  static TetrisInfo_t tetris_info = {0};
  if (ptr_tetris_info == NULL) {
    for (int i = 0; i < FIG_ROWS; i++) {
      tetris_info.next.row[i] = tetris_info.next.cell[i];
    }
    for (int i = 0; i < ROWS; i++) {
      tetris_info.field.row[i] = tetris_info.field.cell[i];
    }
    ptr_tetris_info = &tetris_info;
  }
  return ptr_tetris_info;
}

GameInfo_t *getGameInfo() {
  static GameInfo_t *ptr_game_info = NULL;
  static GameInfo_t game_info;
  if (ptr_game_info == NULL) {
    TetrisInfo_t *ptr_tetris_info = getTetrisInfo();
    game_info.field = (int **)ptr_tetris_info->field.row;
    game_info.next = (int **)ptr_tetris_info->next.row;

    ptr_game_info = &game_info;
  }
  return ptr_game_info;
}

void game() {
  GameInfo_t *info = getGameInfo();
  info->level = 1;
}

void userInput(UserAction_t action, bool hold) { game(); }

GameInfo_t updateCurrentState() { return *getGameInfo(); }

void showState(GameInfo_t info) {
  int line = 1;
  mvprintw(line++, 0, "Level: %d", info.level);
  mvprintw(line++, 0, "Score: %d", info.score);
  mvprintw(line++, 0, "High score: %d", info.high_score);
  mvprintw(line++, 0, "Speed: %d", info.speed);
  mvprintw(line++, 0, "Next:");
  for (int i = 0; i < FIG_ROWS; i++, line++) {
    for (int j = 0; j < FIG_COLS; j++) {
      mvprintw(line, j * 2, "%s", info.next[i][j] ? "[]" : " .");
    }
  }
  mvprintw(line++, 0, "Field:");
  for (int i = 0; i < ROWS; i++, line++) {
    for (int j = 0; j < COLS; j++) {
      mvprintw(line, j * 2, "%s", info.field[i][j] ? "[]" : " .");
    }
  }
}

void gameCycle() {
  // mvaddstr(0, 0, "Menu");
  UserAction_t action;
  bool hold;
  GameInfo_t info;
  int key;
  while ((key = getch()) != KEY_ESCAPE) {
    switch (key) {
      case 10:  // Enter on the regular keyboard
        action = Action;
        break;
      case KEY_LEFT:
        action = Left;
        break;
    }
    userInput(action, hold);
    info = updateCurrentState();
    showState(info);
  }
}

int main() {
  WIN_INIT();
  gameCycle();
  endwin();
  return 0;
}