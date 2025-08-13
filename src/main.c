#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define UPDATE_INTERVAL_MS 1000

#define KEY_ESCAPE 27
#define ENTER_KEY 10
#define KEY_SPACE 32
#define KEY_Q_LOWER 113

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

typedef enum {
  kFigureI,
  kFigureL,
  kFigureO,
  kFigureT,
  kFigureS,
  kFigureZ,
  kFigureJ
} Tetromino_t;

typedef struct {
  struct {
    int *row[ROWS];
    int cell[ROWS][COLS];
  } field;

  struct {
    Tetromino_t type;
    int *row[FIG_ROWS];
    int cell[FIG_ROWS][FIG_COLS];
  } next;

  struct {
    Tetromino_t type;
    Point_t coordinate;
    int cell[FIG_ROWS][FIG_COLS];
    int offset_x;
    int offset_y;
    int rotation;
    int hash_all_rotation;
  } curr_fig;

  UserAction_t action;
  bool hold;
  int score;
  int level;
  unsigned long last_tick;
} TetrisInfo_t;

typedef enum {
  kStart,
  kPause,
  kTerminate,
  kSpawn,
  kMoving,
  kRotating,
  kShifting,
  kAttaching,
  kGameOver
} TetrisState_t;

void userInput(UserAction_t action, bool hold);
TetrisInfo_t *getTetrisInfo();
GameInfo_t *getGameInfo();

void userInput(UserAction_t action, bool hold);
GameInfo_t updateCurrentState();
void showState(GameInfo_t info);
void gameCycle();
unsigned long currentTimeMs();

void onStartState(UserAction_t action);
void onPauseState(UserAction_t action);
void onShiftingState(UserAction_t action);
void onGameOverState(UserAction_t action);

void handleSpawnState();
void handleAttachingState();
bool checkGameOver();

void debugWhichState(TetrisState_t *ptr_state, char *buffer);

void eraseCurrentFigureOnField();
bool coordinateInField(const int x, const int y);
bool figureCannotMove(const int x, const int y, const int field_cell);

bool tryMoveFigure(UserAction_t action);
bool tryRotateFigure();
void rotateFigureI();
void rotateFigureL();
void rotateFigureT();
void rotateFigureS();
void rotateFigureZ();
void rotateFigureJ();

void init_ncurses();

void init_ncurses() {
  initscr();
  cbreak();
  keypad(stdscr, true);
  noecho();
  curs_set(0);
  timeout(1000);
}

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

bool coordinateInField(const int x, const int y) {
  return x >= 0 && x < COLS && y >= 0 && y < ROWS;
}

bool figureCannotMove(const int x, const int y, const int field_cell) {
  return x < 0 || x >= COLS || y >= ROWS || field_cell == 1;
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
  TetrisInfo_t *game = getTetrisInfo();
  int tetrominoes[7][FIG_ROWS][FIG_COLS] = {
      {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},   // kFigureI
      {{0, 0, 0, 0}, {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},   // kFigureL
      {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   // kFigureO
      {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},   // kFigureT
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},   // kFigureS
      {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},   // kFigureZ
      {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}}};  // kFigureJ

  // Generate next tetromino if empty (start of game)
  static bool next_empty = true;
  Tetromino_t type = rand() % 7;
  if (next_empty) {
    copyTetromino(game->next.cell, tetrominoes[type]);
    game->next.type = type;
    type = rand() % 7;
    next_empty = false;
  }
  copyTetromino(game->curr_fig.cell, game->next.cell);
  game->curr_fig.type = game->next.type;

  // Set starting coordinate (top center)
  game->curr_fig.coordinate.x = 3;
  game->curr_fig.coordinate.y = -3;

  // Generate new next tetromino
  copyTetromino(game->next.cell, tetrominoes[type]);
  game->next.type = type;
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

void onPauseState(UserAction_t action) {
  switch (action) {
    case Action:
      setState(kMoving);
      break;
    case Terminate:
      setState(kTerminate);
      break;
  }
}

bool checkGameOver() {
  TetrisInfo_t *game = getTetrisInfo();
  bool game_over = false;
  // If figure was attched in row 0
  if (game->curr_fig.coordinate.y == -2) {  // need fix
    // Game over
    game_over = true;
  }
  return game_over;
}

bool isLineFill(int line);

bool isLineFill(int line) {
  TetrisInfo_t *game = getTetrisInfo();
  bool line_is_fill = true;
  for (int j = 0; j < COLS && line_is_fill; j++) {
    if (game->field.cell[line][j] == false) {
      line_is_fill = false;
    }
  }
  return line_is_fill;
}

void moveGroundDown(int line);

void moveGroundDown(int line) {
  TetrisInfo_t *game = getTetrisInfo();
  for (int i = line; i > 0; i--) {
    for (int j = 0; j < COLS; j++) {
      game->field.cell[i][j] = game->field.cell[i - 1][j];
    }
  }
  for (int j = 0; j < COLS; j++) {
    game->field.cell[0][j] = 0;
  }
}

void handleAttachingState() {
  TetrisInfo_t *game = getTetrisInfo();
  int count_filled_lines = 0;
  // Check filled lines
  for (int line = 0; line < ROWS; line++) {
    if (isLineFill(line)) {
      count_filled_lines += 1;
      // Shift the top pixels to their place
      moveGroundDown(line);
    }
  }

  // Earn points              // bonus part 2
  switch (count_filled_lines) {
    case 1:
      game->score += 100;
      break;
    case 2:
      game->score += 300;
      break;
    case 3:
      game->score += 700;
      break;
    case 4:
      game->score += 1500;
      break;
  }
  // Set new level necessary  // bonus part 3
  game->level = game->score / 600;

  // Set new speed necessary  // bonus part 3

  game->curr_fig.hash_all_rotation = 0;
  game->curr_fig.rotation = 0;
}

void onGameOverState(UserAction_t action) {
  switch (action) {
    case Start:
      // TODO: Clear TetrisInfo
      setState(kSpawn);
      break;
    case Terminate:
      setState(kTerminate);
      break;
  }
}

void onMovingState(UserAction_t action) {
  switch (action) {
    case Left:
      tryMoveFigure(action);
      break;
    case Right:
      tryMoveFigure(action);
      break;
    case Down:
      tryMoveFigure(action);
      break;
    case Action:
      setState(kRotating);
      break;
    case Terminate:
      setState(kTerminate);
      break;
    case Pause:
      setState(kPause);
      break;
  }
}

TetrisInfo_t *getTetrisInfo() {
  static TetrisInfo_t *game = NULL;
  static TetrisInfo_t tetris_info = {0};
  if (game == NULL) {
    for (int i = 0; i < FIG_ROWS; i++) {
      tetris_info.next.row[i] = tetris_info.next.cell[i];
    }
    for (int i = 0; i < ROWS; i++) {
      tetris_info.field.row[i] = tetris_info.field.cell[i];
    }
    game = &tetris_info;
  }
  return game;
}

GameInfo_t *getGameInfo() {
  TetrisInfo_t *game = getTetrisInfo();
  static GameInfo_t game_info;
  static GameInfo_t *ptr_game_info = NULL;
  if (ptr_game_info == NULL) {
    game_info.field = (int **)game->field.row;
    game_info.next = (int **)game->next.row;

    ptr_game_info = &game_info;
  }
  game_info.score = game->score;
  game_info.level = game->level;
  return ptr_game_info;
}

bool tryMoveFigure(UserAction_t action) {
  TetrisInfo_t *game = getTetrisInfo();

  game->curr_fig.offset_x -= (action == Left);
  game->curr_fig.offset_x += (action == Right);
  game->curr_fig.offset_y = (action == Down);

  int x = game->curr_fig.coordinate.x;
  int y = game->curr_fig.coordinate.y;
  int offset_x = game->curr_fig.offset_x;
  int offset_y = game->curr_fig.offset_y;

  // Erase current position
  eraseCurrentFigureOnField();

  // Check new position
  bool can_move = true;
  for (int i = 0; i < FIG_ROWS && can_move; i++) {
    for (int j = 0; j < FIG_COLS && can_move; j++) {
      if (game->curr_fig.cell[i][j]) {
        int new_x = x + j + offset_x;
        int new_y = y + i + offset_y;
        if (figureCannotMove(new_x, new_y, game->field.cell[new_y][new_x])) {
          can_move = false;
        }
      }
    }
  }

  if (can_move) {
    // Add figure to new position
    for (int i = 0; i < FIG_ROWS; i++) {
      for (int j = 0; j < FIG_COLS; j++) {
        if (game->curr_fig.cell[i][j]) {
          int fx = x + j + offset_x;
          int fy = y + i + offset_y;
          if (coordinateInField(fx, fy)) {
            game->field.cell[fy][fx] = game->curr_fig.cell[i][j];
          }
        }
      }
    }
    game->curr_fig.coordinate.x += offset_x;
    game->curr_fig.coordinate.y += offset_y;
    setState(kMoving);

  } else {
    // Turn back last position
    for (int i = 0; i < FIG_ROWS; i++) {
      for (int j = 0; j < FIG_COLS; j++) {
        if (game->curr_fig.cell[i][j]) {
          int fx = x + j;
          int fy = y + i;
          if (coordinateInField(fx, fy)) {
            game->field.cell[fy][fx] = game->curr_fig.cell[i][j];
          }
        }
      }
    }
  }
  game->curr_fig.offset_x = 0;
  game->curr_fig.offset_y = 0;
  return can_move;
}

void rotateCurrentFigure();

void rotateCurrentFigure() {
  TetrisInfo_t *game = getTetrisInfo();
  game->curr_fig.rotation = game->curr_fig.hash_all_rotation % 4;
  switch (game->curr_fig.type) {
    case kFigureI:
      rotateFigureI();
      break;
    case kFigureL:
      rotateFigureL();
      break;
    case kFigureT:
      rotateFigureT();
      break;
    case kFigureS:
      rotateFigureS();
      break;
    case kFigureZ:
      rotateFigureZ();
      break;
    case kFigureJ:
      rotateFigureJ();
      break;
  }
}

void eraseCurrentFigureOnField() {
  TetrisInfo_t *game = getTetrisInfo();

  for (int i = 0; i < FIG_ROWS; i++) {
    for (int j = 0; j < FIG_COLS; j++) {
      if (game->curr_fig.cell[i][j]) {
        int fx = game->curr_fig.coordinate.x + j;
        int fy = game->curr_fig.coordinate.y + i;
        if (coordinateInField(fx, fy)) {
          game->field.cell[fy][fx] = 0;
        }
      }
    }
  }
}

bool tryRotateFigure() {
  // Нужно менять логику расположения на поле
  TetrisInfo_t *game = getTetrisInfo();

  int x = game->curr_fig.coordinate.x;
  int y = game->curr_fig.coordinate.y;

  // Erase current position
  eraseCurrentFigureOnField();

  game->curr_fig.hash_all_rotation += 1;
  rotateCurrentFigure();

  // Check new position
  bool can_move = true;
  for (int i = 0; i < FIG_ROWS && can_move; i++) {
    for (int j = 0; j < FIG_COLS && can_move; j++) {
      if (game->curr_fig.cell[i][j]) {
        int new_x = x + j;
        int new_y = y + i;
        if (figureCannotMove(new_x, new_y, game->field.cell[new_y][new_x])) {
          can_move = false;
        }
      }
    }
  }

  if (can_move) {
    // Add figure to new position
    for (int i = 0; i < FIG_ROWS; i++) {
      for (int j = 0; j < FIG_COLS; j++) {
        if (game->curr_fig.cell[i][j]) {
          int fx = x + j;
          int fy = y + i;
          if (coordinateInField(fx, fy)) {
            game->field.cell[fy][fx] = game->curr_fig.cell[i][j];
          }
        }
      }
    }
  } else {
    // Turn back last position

    game->curr_fig.hash_all_rotation -= 1;
    rotateCurrentFigure();

    for (int i = 0; i < FIG_ROWS; i++) {
      for (int j = 0; j < FIG_COLS; j++) {
        if (game->curr_fig.cell[i][j]) {
          int fx = x + j;
          int fy = y + i;
          if (coordinateInField(fx, fy)) {
            game->field.cell[fy][fx] = game->curr_fig.cell[i][j];
          }
        }
      }
    }
  }
  return can_move;
}

// Clear previos pixels and set new
void rotateFigureI() {
  TetrisInfo_t *game = getTetrisInfo();
  switch (game->curr_fig.rotation) {
    case 1:
      game->curr_fig.cell[0][0] = 0;  //  . .[] .
      game->curr_fig.cell[0][1] = 0;  //  . .[] .
      game->curr_fig.cell[0][2] = 1;  //  . .[] .
      game->curr_fig.cell[0][3] = 0;  //  . .[] .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 0;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 1;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 1;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 2:
      game->curr_fig.cell[0][0] = 0;  //  . . . .
      game->curr_fig.cell[0][1] = 0;  //  . . . .
      game->curr_fig.cell[0][2] = 0;  // [][][][]
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 0;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 1;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 1;
      game->curr_fig.cell[2][3] = 1;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 3:
      game->curr_fig.cell[0][0] = 0;  //  .[] . .
      game->curr_fig.cell[0][1] = 1;  //  .[] . .
      game->curr_fig.cell[0][2] = 0;  //  .[] . .
      game->curr_fig.cell[0][3] = 0;  //  .[] . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 1;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 0:
      game->curr_fig.cell[0][0] = 0;  //  . . . .
      game->curr_fig.cell[0][1] = 0;  // [][][][]
      game->curr_fig.cell[0][2] = 0;  //  . . . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 1;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
  }
}
void rotateFigureJ() {
  TetrisInfo_t *game = getTetrisInfo();
  switch (game->curr_fig.rotation) {
    case 1:
      game->curr_fig.cell[0][0] = 0;  //  .[] . .
      game->curr_fig.cell[0][1] = 1;  //  .[] . .
      game->curr_fig.cell[0][2] = 0;  // [][] . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 1;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 2:
      game->curr_fig.cell[0][0] = 1;  // [] . . .
      game->curr_fig.cell[0][1] = 0;  // [][][] .
      game->curr_fig.cell[0][2] = 0;  //  . . . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 3:
      game->curr_fig.cell[0][0] = 0;  //  .[][] .
      game->curr_fig.cell[0][1] = 1;  //  .[] . .
      game->curr_fig.cell[0][2] = 1;  //  .[] . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 0:
      game->curr_fig.cell[0][0] = 0;  //  . . . .
      game->curr_fig.cell[0][1] = 0;  // [][][] .
      game->curr_fig.cell[0][2] = 0;  //  . .[] .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 1;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
  }
}
void rotateFigureT() {
  TetrisInfo_t *game = getTetrisInfo();
  switch (game->curr_fig.rotation) {
    case 1:
      game->curr_fig.cell[0][0] = 0;  //  .[] . .
      game->curr_fig.cell[0][1] = 1;  // [][] . .
      game->curr_fig.cell[0][2] = 0;  //  .[] . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 2:
      game->curr_fig.cell[0][0] = 0;  //  .[] . .
      game->curr_fig.cell[0][1] = 1;  // [][][] .
      game->curr_fig.cell[0][2] = 0;  //  . . . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 3:
      game->curr_fig.cell[0][0] = 0;  //  .[] . .
      game->curr_fig.cell[0][1] = 1;  //  .[][] .
      game->curr_fig.cell[0][2] = 0;  //  .[] . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 0:
      game->curr_fig.cell[0][0] = 0;  //  . . . .
      game->curr_fig.cell[0][1] = 0;  // [][][] .
      game->curr_fig.cell[0][2] = 0;  //  .[] . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
  }
}
void rotateFigureS() {
  TetrisInfo_t *game = getTetrisInfo();
  switch (game->curr_fig.rotation) {
    case 1:
      game->curr_fig.cell[0][0] = 1;  // [] . . .
      game->curr_fig.cell[0][1] = 0;  // [][] . .
      game->curr_fig.cell[0][2] = 0;  //  .[] . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 2:
      game->curr_fig.cell[0][0] = 0;  //  .[][] .
      game->curr_fig.cell[0][1] = 1;  // [][] . .
      game->curr_fig.cell[0][2] = 1;  //  . . . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 3:
      game->curr_fig.cell[0][0] = 0;  //  .[] . .
      game->curr_fig.cell[0][1] = 1;  //  .[][] .
      game->curr_fig.cell[0][2] = 0;  //  . .[] .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 1;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 0:
      game->curr_fig.cell[0][0] = 0;  //  . . . .
      game->curr_fig.cell[0][1] = 0;  //  .[][] .
      game->curr_fig.cell[0][2] = 0;  // [][] . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 1;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
  }
}
void rotateFigureZ() {
  TetrisInfo_t *game = getTetrisInfo();
  switch (game->curr_fig.rotation) {
    case 1:
      game->curr_fig.cell[0][0] = 0;  //  .[] . .
      game->curr_fig.cell[0][1] = 1;  // [][] . .
      game->curr_fig.cell[0][2] = 0;  // [] . . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 1;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 2:
      game->curr_fig.cell[0][0] = 1;  // [][] . .
      game->curr_fig.cell[0][1] = 1;  //  .[][] .
      game->curr_fig.cell[0][2] = 0;  //  . . . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 3:
      game->curr_fig.cell[0][0] = 0;  //  . .[] .
      game->curr_fig.cell[0][1] = 0;  //  .[][] .
      game->curr_fig.cell[0][2] = 1;  //  .[] . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 0:
      game->curr_fig.cell[0][0] = 0;  //  . . . .
      game->curr_fig.cell[0][1] = 0;  // [][] . .
      game->curr_fig.cell[0][2] = 0;  //  .[][] .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 1;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
  }
}
void rotateFigureL() {
  TetrisInfo_t *game = getTetrisInfo();
  switch (game->curr_fig.rotation) {
    case 1:
      game->curr_fig.cell[0][0] = 1;  // [][] . .
      game->curr_fig.cell[0][1] = 1;  //  .[] . .
      game->curr_fig.cell[0][2] = 0;  //  .[] . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 2:
      game->curr_fig.cell[0][0] = 0;  //  . .[] .
      game->curr_fig.cell[0][1] = 0;  // [][][] .
      game->curr_fig.cell[0][2] = 1;  //  . . . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 3:
      game->curr_fig.cell[0][0] = 0;  //  .[] . .
      game->curr_fig.cell[0][1] = 1;  //  .[] . .
      game->curr_fig.cell[0][2] = 0;  //  .[][] .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 0;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 0;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 0;
      game->curr_fig.cell[2][1] = 1;
      game->curr_fig.cell[2][2] = 1;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
    case 0:
      game->curr_fig.cell[0][0] = 0;  //  . . . .
      game->curr_fig.cell[0][1] = 0;  // [][][] .
      game->curr_fig.cell[0][2] = 0;  // [] . . .
      game->curr_fig.cell[0][3] = 0;  //  . . . .
      game->curr_fig.cell[1][0] = 1;
      game->curr_fig.cell[1][1] = 1;
      game->curr_fig.cell[1][2] = 1;
      game->curr_fig.cell[1][3] = 0;
      game->curr_fig.cell[2][0] = 1;
      game->curr_fig.cell[2][1] = 0;
      game->curr_fig.cell[2][2] = 0;
      game->curr_fig.cell[2][3] = 0;
      game->curr_fig.cell[3][0] = 0;
      game->curr_fig.cell[3][1] = 0;
      game->curr_fig.cell[3][2] = 0;
      game->curr_fig.cell[3][3] = 0;
      break;
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
    case KEY_LEFT:  // Move figure to left
      action = Left;
      break;
    case KEY_RIGHT:  // Move figure to right
      action = Right;
      break;
    case KEY_UP:
      action = Action;  // Rotate the figure
      break;
    case KEY_SPACE:
      action = Up;  // TODO: CHANGE WITH KEY_UP
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

bool timeToShift() {
  TetrisInfo_t *game = getTetrisInfo();
  unsigned long now = currentTimeMs();
  if (now - game->last_tick >= UPDATE_INTERVAL_MS) {
    game->last_tick = now;
    return true;
  }
  return false;
}

void userInput(UserAction_t action, bool hold) {
  TetrisState_t state = *getState();
  switch (state) {
    case kStart:
      onStartState(action);
      break;
    case kPause:
      onPauseState(action);
      break;
    case kMoving:
      onMovingState(action);
      break;
    case kGameOver:
      onGameOverState(action);
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
    case kShifting:
      if (tryMoveFigure(Down)) {
        setState(kMoving);
      } else {
        setState(kAttaching);
      }
      break;
    case kMoving:
      if (timeToShift()) {
        setState(kShifting);
      }
      break;
    case kAttaching:
      if (checkGameOver()) {
        setState(kGameOver);
      } else {
        handleAttachingState();
        setState(kSpawn);
      }
      break;
    case kRotating:
      tryRotateFigure();
      if (timeToShift()) {
        setState(kShifting);
      } else {
        setState(kMoving);
      }
      break;
  }
  return *getGameInfo();
}

// BrickGame function
void showState(GameInfo_t info) {
  int line = 0;

#ifdef DEBUG
  TetrisState_t *ptr_state = getState();
  TetrisInfo_t *ptr_info = getTetrisInfo();
  char buffer[15] = {0};
  char prev_buffer[15] = {0};
  debugWhichState(ptr_state, buffer);
  mvprintw(line, 0, "State: %s", "               ");
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
  for (int i = 0; i < FIG_ROWS; i++, line++) {
    for (int j = 0; j < FIG_COLS; j++) {
      mvprintw(line, j * 2, "%s", info.next[i][j] ? "[]" : " .");
    }
  }
  mvprintw(line++, 0, "Field:");
  for (int i = 0; i < ROWS; i++, line++) {
#ifdef DEBUG
    if (i == 0) {
      for (int j = 0; j < COLS; j++) {
        mvprintw(line, j * 2, "%2d", j);
      }
      line++;
    }
    mvprintw(line, 21, "%d", field_line++);
#endif  // DEBUG
    for (int j = 0; j < COLS; j++) {
      mvprintw(line, j * 2, "%s", info.field[i][j] ? "[]" : " .");
    }
  }
}

// DEBUG function
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

unsigned long currentTimeMs() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (unsigned long)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// BrickGame function
void gameCycle() {
  UserAction_t action = Up;
  bool hold;
  GameInfo_t info;
  TetrisState_t *ptr_state = getState();
  unsigned long last_update = 0;
  bool run_game = true;
  while (run_game) {
    if (*ptr_state == kTerminate) {
      run_game = false;
    }

    userInput(action, hold);
    info = updateCurrentState();
    showState(info);
    if (*ptr_state == kMoving || *ptr_state == kStart ||
        *ptr_state == kGameOver || *ptr_state == kPause) {
      action = getSignal();
    }
    usleep(10000);
  }
}

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