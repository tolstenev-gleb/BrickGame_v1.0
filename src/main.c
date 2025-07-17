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


GameInfo_t *getInfo() {
  static GameInfo_t info;
  return &info;
}

void game() {
  GameInfo_t *info = getInfo();
  info->level = 1;
}

void userInput(UserAction_t action, bool hold);

void userInput(UserAction_t action, bool hold) {
  if (action == Action) {
    game();
  }
}


GameInfo_t updateCurrentState() {
  return *getInfo();
}

void showState(GameInfo_t info) {
  mvprintw(0, 0, "Level: %d", info.level);
}

void view() {
  mvaddstr(0, 0, "Menu");
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
  view();
  endwin();
  return 0;
}