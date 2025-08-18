#include "tetris.h"


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
  static int tetrominoes[7][FIG_ROWS][FIG_COLS] = {
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
  // game->curr_fig.coordinate.y = game->curr_fig.type == kFigureI ? -3 : -2;
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
    default:
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
    default:
      break;
  }
}

int getLowestCoordinate() {
  TetrisInfo_t *game = getTetrisInfo();
  int lowest_row = 0;
  for (int i = FIG_ROWS - 1; i > 0 && lowest_row == 0; i--) {
    for (int j = 0; j < FIG_COLS && lowest_row == 0; j++) {
      if (game->curr_fig.cell[i][j]) {
        lowest_row = i;
      }
    }
  }
  return game->curr_fig.coordinate.y + lowest_row;
}

bool checkGameOver() {
  bool game_over = false;
  // If figure was attched in row 0
  if (getLowestCoordinate() <= 0) {
    // Game over
    game_over = true;
  }
  return game_over;
}

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
  if (game->score > game->high_score) {
    game->high_score = game->score;
  }
  // Set new level necessary  // bonus part 3
  if (count_filled_lines > 0 && game->level < 10) {
    game->level = game->score / 600;
    if (game->level > 10) {
      game->level = 10;
    }
    // Set new speed necessary  // bonus part 3
    game->speed = game->level;
    game->update_interval = 1000 - game->speed * 75;
  }

  game->curr_fig.hash_all_rotation = 0;
  game->curr_fig.rotation = 0;
}

void clearArray(int **array, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      array[i][j] = 0;
    }
  }
}

void clearTetrisInfo() {
  TetrisInfo_t *game = getTetrisInfo();
  game->last_tick = currentTimeMs();
  game->level = 0;
  game->score = 0;
  clearArray(game->curr_fig.row, FIG_ROWS, FIG_COLS);
  clearArray(game->field.row, ROWS, COLS);
}

void onGameOverState(UserAction_t action) {
  switch (action) {
    case Start:
      clearTetrisInfo();
      setState(kSpawn);
      break;
    case Terminate:
      setState(kTerminate);
      break;
    default:
      break;
  }
}

void dropFigure() {
  while (tryMoveFigure(Down) == true) {
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
      dropFigure();
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
    default:
      break;
  }
}

TetrisInfo_t *initTetrisInfo() {
  static TetrisInfo_t game = {0};
  game.update_interval = 1000;
  for (int i = 0; i < FIG_ROWS; i++) {
    game.next.row[i] = game.next.cell[i];
    game.curr_fig.row[i] = game.curr_fig.cell[i];
  }
  for (int i = 0; i < ROWS; i++) {
    game.field.row[i] = game.field.cell[i];
  }
  FILE *file = fopen("highscore.txt", "r");
  if (file) {
    char score[10] = {'\0'};
    fgets(score, sizeof(score), file);
    game.high_score = atoi(score);
    fclose(file);
  }
  return &game;
}

TetrisInfo_t *getTetrisInfo() {
  static TetrisInfo_t *game = NULL;
  if (game == NULL) {
    game = initTetrisInfo();
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
  game_info.speed = game->speed;
  game_info.score = game->score;
  game_info.high_score = game->high_score;
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
    case kFigureO:
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

bool timeToShift() {
  TetrisInfo_t *game = getTetrisInfo();
  unsigned long now = currentTimeMs();
  if (now - game->last_tick >= game->update_interval) {
    game->last_tick = now;
    return true;
  }
  return false;
}

void saveHighScore() {
  TetrisInfo_t *game = getTetrisInfo();
  FILE *file = fopen("highscore.txt", "w");
  if (file) {
    fprintf(file, "%d", game->high_score);
    fclose(file);
  }
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
    default:
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
      handleAttachingState();
      if (checkGameOver()) {
        setState(kGameOver);
      } else {
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
    case kTerminate:
      saveHighScore();
      break;
    default:
      break;
  }
  return *getGameInfo();
}

unsigned long currentTimeMs() {
  struct timespec ts;
  clock_gettime(1, &ts);
  return (unsigned long)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// BrickGame function
