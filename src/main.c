#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define WIN_INIT()        \
  {                       \
    initscr();            \
    cbreak();             \
    timeout(100);         \
    keypad(stdscr, true); \
    noecho();             \
    curs_set(0);          \
    srand(time(NULL));    \
  }

#define KEY_ESCAPE 27
#define ENTER_KEY 10
#define KEY_SPACE 32
#define KEY_Q 81

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
  int x;
  int y;
} Point_t;

typedef struct {
  struct {
    int *row[ROWS];
    int cell[ROWS][COLS];
  } field;
  struct {
    int *row[FIG_ROWS];
    int cell[FIG_ROWS][FIG_COLS];
  } next;
  struct {
    int cell[FIG_ROWS][FIG_COLS];
    Point_t coordinate;
  } curr_fig;
  UserAction_t action;
  bool hold;
} TetrisInfo_t;

typedef enum {
  kStart,
  kPause,
  kTerminate,
  kSpawn,
  kMoving,
  kShifting,
  kAttaching,
  kGameOver
} TetrisState_t;

TetrisState_t *initState() {
  static TetrisState_t state = kStart;
  return &state;
}

TetrisState_t *getState() {
  static TetrisState_t *ptr_state = NULL;
  if (ptr_state == NULL) {
    ptr_state = initState();
  }
  return ptr_state;
}

void setState(TetrisState_t new_state) {
  TetrisState_t *ptr_state = getState();
  *ptr_state = new_state;
}

void userInput(UserAction_t action, bool hold);
TetrisInfo_t *getTetrisInfo();
GameInfo_t *getGameInfo();
void game();
void userInput(UserAction_t action, bool hold);
GameInfo_t updateCurrentState();
void showState(GameInfo_t info);
void gameCycle();

void onStartState();
void onPauseState();
void onExitState();
// void onSpawnState();
void onMovingState();
void onShiftingState();
void onAttachingState();
void onGameOverState();

void handleSpawnState();
void handleShiftingState();

void handleShiftingState() {
  TetrisInfo_t *ptr_tetris_info = getTetrisInfo();
  if (ptr_tetris_info->curr_fig.coordinate.y < 18) {
  ptr_tetris_info->curr_fig.coordinate.y += 1;
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      ptr_tetris_info->field.cell[i][j] = 0;
    }
  }
  for (int i = 0; i < FIG_ROWS; i++) {
    for (int j = 0; j < FIG_COLS; j++) {
      if (ptr_tetris_info->curr_fig.cell[i][j]) {
        int fx = ptr_tetris_info->curr_fig.coordinate.x + j;
        int fy = ptr_tetris_info->curr_fig.coordinate.y + i;
        if (fx >= 0 && fx < COLS && fy >= 0 && fy < ROWS)
          ptr_tetris_info->field.cell[fy][fx] =
              ptr_tetris_info->curr_fig.cell[i][j];
      }
    }
  } }
  else {
    setState(kSpawn);
  }
}

void copyTetromino(int dst_fig[FIG_ROWS][FIG_COLS],
                   int src_fig[FIG_ROWS][FIG_COLS]) {
  for (int i = 0; i < FIG_ROWS; i++) {
    for (int j = 0; j < FIG_COLS; j++) {
      dst_fig[i][j] = src_fig[i][j];
    }
  }
}

void handleSpawnState() {
  TetrisInfo_t *ptr_tetris_info = getTetrisInfo();
  int tetrominos[7][FIG_ROWS][FIG_COLS] = {
      {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},   // I
      {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   // O
      {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   // T
      {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   // S
      {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   // Z
      {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   // J
      {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};  // L

  // Generate next tetromino if empty (start of game)
  static bool next_empty = true;
  if (next_empty) {
    copyTetromino(ptr_tetris_info->next.cell, tetrominos[rand() % 7]);
    next_empty = false;
  }
  copyTetromino(ptr_tetris_info->curr_fig.cell, ptr_tetris_info->next.cell);

  // Set starting coordinate (top center)
  ptr_tetris_info->curr_fig.coordinate.x = COLS / 2 - 2;
  ptr_tetris_info->curr_fig.coordinate.y = 0;

  // Place curr_fig into field

  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      ptr_tetris_info->field.cell[i][j] = 0;
    }
  }

  for (int i = 0; i < FIG_ROWS; i++) {
    for (int j = 0; j < FIG_COLS; j++) {
      if (ptr_tetris_info->curr_fig.cell[i][j]) {
        int fx = ptr_tetris_info->curr_fig.coordinate.x + j;
        int fy = ptr_tetris_info->curr_fig.coordinate.y + i;
        if (fx >= 0 && fx < COLS && fy >= 0 && fy < ROWS)
          ptr_tetris_info->field.cell[fy][fx] =
              ptr_tetris_info->curr_fig.cell[i][j];
      }
    }
  }

  // Generate new next tetromino
  copyTetromino(ptr_tetris_info->next.cell, tetrominos[rand() % 7]);
}

void onStartState(UserAction_t action) {
  switch (action) {
    case Start:
      setState(kSpawn);
      break;
    case Terminate:
      setState(kTerminate);
      break;
  }
}
void onPauseState() {}
void onExitState() {}
void onSpawnState() {}
void onMovingState() {}
void onShiftingState(UserAction_t action) {
  switch (action) {
#ifdef DEBUG
    case Start:
      setState(kSpawn);
      break;
#endif
    case Left:
      break;
  }
}
void onAttachingState() {}
void onGameOverState() {}

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

void userInput(UserAction_t action, bool hold) {
  TetrisState_t state = *getState();
  switch (state) {
    case kStart:
      onStartState(action);
      break;
    case kPause:
      onPauseState();
      break;
    case kTerminate:
      onExitState();
      break;
    case kMoving:
      onMovingState();
      break;
    case kShifting:
      onShiftingState(action);
      break;
    case kAttaching:
      onAttachingState();
      break;
    case kGameOver:
      onGameOverState();
      break;
  }
}

GameInfo_t updateCurrentState() {
  TetrisState_t state = *getState();
  switch (state) {
    case kSpawn:
      handleSpawnState();
      setState(kShifting);
      break;
    case kMoving:
      // handleSpawnState();
      break;
    case kShifting:
      handleShiftingState();
      break;
      /*
        case :
          handle();
          break;
        */
  }

  return *getGameInfo();
}

// BrickGame function
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

// BrickGame function
UserAction_t getSignal() {
  int signal = getch();
  UserAction_t action = Up;
  switch (signal) {
    case ENTER_KEY:  // Start game
      action = Start;
      break;
    case KEY_SPACE:  // Rotate the figure
      action = Action;
      break;
    case KEY_LEFT:  // Move figure to left
      action = Left;
      break;
    case KEY_RIGHT:  // Move figure to right
      action = Right;
      break;
    case KEY_UP:
      // action = Up; don't use in Tetris
      break;
    case KEY_DOWN:  // The falling of figure
      action = Down;
      break;
    case KEY_ESCAPE:  // Pause the game
      action = Pause;
      break;
    case KEY_Q:  // Quit from the game
      action = Terminate;
      break;
  }
  return action;
}

// DEBUG function
void debugWhichState(TetrisState_t *ptr_state, char *buffer) {
  switch (*ptr_state) {
    case kStart:
      strcpy(buffer, "Start");
      break;
    case kPause:
      strcpy(buffer, "Pause");
      break;
    case kTerminate:
      strcpy(buffer, "Exit");
      break;
    case kSpawn:
      strcpy(buffer, "Spawn");
      break;
    case kMoving:
      strcpy(buffer, "Moving");
      break;
    case kShifting:
      strcpy(buffer, "Shifting");
      break;
    case kAttaching:
      strcpy(buffer, "Attaching");
      break;
    case kGameOver:
      strcpy(buffer, "GameOver");
      break;
  }
}

// BrickGame function
void gameCycle() {
  UserAction_t action = Up;
  bool hold;
  GameInfo_t info;
  TetrisState_t *ptr_state = getState();

  bool run_game = true;
  while (run_game) {
    if (*ptr_state == kGameOver || *ptr_state == kTerminate) {
      run_game = false;
    }

    userInput(action, hold);
    info = updateCurrentState();

#ifdef DEBUG
    char buffer[15] = {0};
    char prev_buffer[15] = {0};
    debugWhichState(ptr_state, buffer);
    mvprintw(0, 0, "State: %s", buffer);
#endif  // DEBUG

    showState(info);

    if (*ptr_state == kShifting || *ptr_state == kStart /* || ... */) {
      action = getSignal();
      // sleep(1);
    }
  }
}

int main() {
  WIN_INIT();
  // run BrickGame
  // Should BrickGame let choose the game?
  // The game was choosed
  gameCycle();
  endwin();
  return 0;
}