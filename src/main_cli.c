#include "gui/cli/cli.h"

int main() {
  initNcurses();
  gameCycle();
  endwin();
  return 0;
}
