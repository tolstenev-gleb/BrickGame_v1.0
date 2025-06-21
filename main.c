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


int main() {
  WIN_INIT();
  addch('A');
  refresh();
  getch();
  endwin();
  return 0;
}