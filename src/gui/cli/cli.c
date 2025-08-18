#include "cli.h"

void initNcurses() {
  initscr();
  cbreak();
  keypad(stdscr, true);
  noecho();
  curs_set(0);
  timeout(100);
}

void gameCycle() {
  UserAction_t action = Up;
  GameInfo_t info;
  bool run_game = true;
  bool hold;
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 100000};  // 100 microsec
  do {
    action = getSignal();
    hold = (action == Down ? true : false);
    userInput(action, hold);
    info = updateCurrentState();
    nanosleep(&ts, NULL);
    run_game = showState(info);
  } while (run_game);
}

bool showState(GameInfo_t info) {
  bool run_game = true;
  if (info.field == NULL && info.next == NULL) {
    run_game = false;
  } else {
    int line = 0;
#ifdef DEBUG
    TetrisState_t *ptr_state = getState();
    TetrisInfo_t *ptr_info = getTetrisInfo();
    char buffer[15] = {0};
    char prev_buffer[15] = {0};
    debugWhichState(ptr_state, buffer);
    mvprintw(line, 0, "State: %s", "                            ");
    mvprintw(line++, 0, "State: %s", buffer);
    mvprintw(line++, 0, "coordinate.y: %d ", ptr_info->curr_fig.coordinate.y);
    mvprintw(line++, 0, "coordinate.x: %d ", ptr_info->curr_fig.coordinate.x);
    int field_line = 0;
#endif  // DEBUG
    mvprintw(line++, 0, "Level: %d", info.level);
    mvprintw(line++, 0, "Score: %d", info.score);
    mvprintw(line++, 0, "High score: %d", info.high_score);
    mvprintw(line++, 0, "Speed: %d", info.speed);
    mvprintw(line++, 0, "Next:");
    for (int i = 0; i < 4; i++, line++) {
      for (int j = 0; j < 4; j++) {
        mvprintw(line, j * 2, "%s", info.next[i][j] ? "[]" : " .");
      }
    }
    mvprintw(line++, 0, "Field:");
    for (int i = 0; i < 20; i++, line++) {
#ifdef DEBUG
      if (i == 0) {
        for (int j = 0; j < 10; j++) {
          mvprintw(line, j * 2, "%2d", j);
        }
        line++;
      }
      mvprintw(line, 21, "%d", field_line++);
#endif  // DEBUG
      for (int j = 0; j < 10; j++) {
        mvprintw(line, j * 2, "%s", info.field[i][j] ? "[]" : " .");
      }
    }
  }
  return run_game;
}

#ifdef DEBUG
void debugWhichState(TetrisState_t *ptr_state, char *buffer) {
  switch (*ptr_state) {
    case kStart:
      strcpy(buffer, "kStart");
      break;
    case kPause:
      strcpy(buffer, "kPause");
      break;
    case kTerminate:
      strcpy(buffer, "kTerminate");
      break;
    case kSpawn:
      strcpy(buffer, "kSpawn");
      break;
    case kMoving:
      strcpy(buffer, "kMoving");
      break;
    case kRotating:
      strcpy(buffer, "kRotating");
      break;
    case kShifting:
      strcpy(buffer, "kShifting");
      break;
    case kAttaching:
      strcpy(buffer, "kAttaching");
      break;
    case kGameOver:
      strcpy(buffer, "kGameOver");
      break;
  }
}
#endif  // DEBUG

// BrickGame function
UserAction_t getSignal() {
  int signal = getch();
  UserAction_t action = Up;
  switch (signal) {
    case ENTER_KEY:  // Start game
      action = Start;
      break;
    case KEY_LEFT:  // Move figure to left
      action = Left;
      break;
    case KEY_RIGHT:  // Move figure to right
      action = Right;
      break;
    case KEY_UP:
      action = Up;  // No effect
      break;
    case KEY_SPACE:
      action = Action;  // Rotate the figure
      break;
    case KEY_DOWN:  // The falling of figure
      action = Down;
      break;
    case KEY_ESCAPE:  // Pause the game
      action = Pause;
      break;
    case KEY_Q_LOWER:  // Quit from the BrickGame
      action = Terminate;
      break;
  }
  return action;
}
