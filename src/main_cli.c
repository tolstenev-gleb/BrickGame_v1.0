#include "gui/cli/cli.h"

int main() {
  initNcurses();
  gameLoop();
  endwin();
  return 0;
}
