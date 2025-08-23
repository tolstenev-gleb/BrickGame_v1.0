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
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 100000};  // 100 microsec
  do {
    action = getSignal();
    userInput(action, false);
    info = updateCurrentState();
    run_game = showState(info);
    nanosleep(&ts, NULL);
  } while (run_game);
}

bool showState(GameInfo_t info) {
  bool run_game = true;
  if (info.field == NULL && info.next == NULL) {
    run_game = false;
  } else {
    int left_line = 0;
    int right_line = 0;
    int left_side = 0;  // coordinate for field and help text
    int right_side = 23;
#ifdef DEBUG
    TetrisState_t *ptr_state = getState();
    TetrisInfo_t *ptr_info = getTetrisInfo();
    char buffer[15] = {0};
    char prev_buffer[15] = {0};
    debugWhichState(ptr_state, buffer);
    mvprintw(left_line, left_side, "State: %s", "                            ");
    mvprintw(left_line++, left_side, "State: %s", buffer);
    mvprintw(left_line++, left_side, "coordinate.y: %d ",
             ptr_info->curr_fig.coordinate.y);
    mvprintw(left_line++, left_side, "coordinate.x: %d ",
             ptr_info->curr_fig.coordinate.x);
    int field_line = 0;
#endif  // DEBUG
    mvprintw(right_line++, right_side, "Score: %d          ", info.score);
    mvprintw(right_line++, right_side, "High score: %d     ", info.high_score);
    mvprintw(right_line++, right_side, "Level: %d          ", info.level);
    mvprintw(right_line++, right_side, "Speed: %d          ", info.speed);
    right_line++;
    mvprintw(right_line++, right_side, "Next:");
    for (int i = 0; i < 4; i++, right_line++) {
      for (int j = 0; j < 4; j++) {
        mvprintw(right_line, right_side + j * 2, "%s",
                 info.next[i][j] ? "[]" : " .");
      }
    }
    for (int i = 0; i < 20; i++, left_line++) {
#ifdef DEBUG
      if (i == 0) {
        for (int j = 0; j < 10; j++) {
          mvprintw(left_line, j * 2, "%2d", j);
        }
        left_line++;
      }
      mvprintw(left_line, 21, "%d", field_line++);
#endif  // DEBUG
      for (int j = 0; j < 10; j++) {
        mvprintw(left_line, left_side + j * 2, "%s",
                 info.field[i][j] ? "[]" : " .");
      }
    }
#ifdef HELP
    left_line++;
    mvprintw(left_line++, left_side, "%s", "'Enter' | start game");
    mvprintw(left_line++, left_side, "%s", " 'Esc'  | pause / unpause");
    mvprintw(left_line++, left_side, "%s", "  'q'   | exit");
    mvprintw(left_line++, left_side, "%s", "'space' | action");
    mvprintw(left_line++, left_side, "%s",
             "'arrows'| move left, right, up, down");
#endif  // #ifdef HELP
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
