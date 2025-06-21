#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#define WIN_INIT()        \
  {                       \
    initscr();            \
    cbreak();             \
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

void userInput(UserAction_t action, bool hold);

GameInfo_t updateCurrentState();

int main() {
  WIN_INIT();
  addch('A');
  refresh();
  getch();
  endwin();
  return 0;
}