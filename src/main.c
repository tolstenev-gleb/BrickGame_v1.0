#include "gui/cli/cli.h"

int main() {
  initNcurses();
  // run BrickGame
  // Should BrickGame let choose the game?
  // The game was choosed
  gameCycle();
  endwin();
  return 0;
}
