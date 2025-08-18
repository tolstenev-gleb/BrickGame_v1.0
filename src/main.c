#include "gui/cli/cli.h"

int main() {
  srand(time(NULL));
  init_ncurses();
  // run BrickGame
  // Should BrickGame let choose the game?
  // The game was choosed
  gameCycle();
  endwin();
  return 0;
}
