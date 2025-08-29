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

TetrisInfo_t *getTetrisInfo() {
  static TetrisInfo_t *game = NULL;
  if (game == NULL) {
    game = initTetrisInfo();
  }
  return game;
}

TetrisInfo_t *initTetrisInfo() {
  srand(time(NULL));
  static TetrisInfo_t game = {.run_game = true, .update_interval = 1000};
  for (int i = 0; i < kFigRows; i++) {
    game.next.fig.row[i] = game.next.fig.cell[i];
    game.current.fig.row[i] = game.current.fig.cell[i];
  }
  for (int i = 0; i < kRows; i++) {
    game.field.row[i] = game.field.cell[i];
  }
  FILE *file = fopen("highscore_tetris.txt", "r");
  if (file) {
    fscanf(file, "%d", &game.high_score);
    fclose(file);
  }
  return &game;
}

GameInfo_t *getGameInfo() {
  TetrisInfo_t *game = getTetrisInfo();
  static GameInfo_t game_info;
  static GameInfo_t *ptr_game_info = NULL;
  if (ptr_game_info == NULL) {
    game_info.field = (int **)game->field.row;
    game_info.next = (int **)game->next.fig.row;

    ptr_game_info = &game_info;
  }
  if (game->run_game) {
    game_info.speed = game->speed;
    game_info.score = game->score;
    game_info.high_score = game->high_score;
    game_info.level = game->level;
    game_info.pause = game->pause;
  } else {
    // Установка NULL спользуется для передачи на интерфейс информации о том,
    // что пользователь хочет выйти из программы BrickGame
    game_info.field = NULL;
    game_info.next = NULL;
  }
  return ptr_game_info;
}

void clearTetrisInfo() {
  TetrisInfo_t *game = getTetrisInfo();
  game->last_tick = currentTimeMs();
  game->level = 0;
  game->speed = 0;
  game->score = 0;
  clearArray(game->current.fig.row, kFigRows, kFigCols);
  clearArray(game->field.row, kRows, kCols);
}

void clearArray(int **array, int rows, int cols) {
  // maybe memset 0 ?
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      array[i][j] = 0;
    }
  }
}

void onMovingState(UserAction_t action) {
  TetrisInfo_t *game = getTetrisInfo();
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
      tryRotateFigure();
      break;
    case Terminate:
      handleTerminateState();
      break;
    case Pause:
      setState(kPause);
      game->pause = 1;
      break;
    default:
      break;
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
  if (state == kMoving && timeToShift()) {
    if (!tryMoveFigure(Down)) {
      handleAttaching();
      if (checkGameOver()) {
        saveHighScore();
        setState(kGameOver);
      } else {
        generateNextFigure();
      }
    }
  }
  return *getGameInfo();
}

void handleTerminateState() {
  TetrisInfo_t *game = getTetrisInfo();
  saveHighScore();
  game->run_game = false;
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

unsigned long currentTimeMs() {
  struct timespec ts;
  clock_gettime(1, &ts);
  return (unsigned long)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void saveHighScore() {
  TetrisInfo_t *game = getTetrisInfo();
  FILE *file = fopen("highscore_tetris.txt", "w");
  if (file) {
    fprintf(file, "%d\n", game->high_score);
    fclose(file);
  }
}

bool coordinateInField(const int x, const int y) {
  return x >= 0 && x < kCols && y >= 0 && y < kRows;
}

bool figureCannotMove(const int x, const int y, const int field_cell) {
  return x < 0 || x >= kCols || y >= kRows || field_cell == 1;
}

void copyTetromino(int dst_fig[kFigRows][kFigCols],
                   int src_fig[kFigRows][kFigCols]) {
  for (int i = 0; i < kFigRows; i++) {
    for (int j = 0; j < kFigCols; j++) {
      dst_fig[i][j] = src_fig[i][j];
    }
  }
}

void setFigure(Figure_t *ptr_fig, Tetromino_t type) {
  static int tetrominoes[7][kFigRows][kFigCols] = {
      {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},   // kFigureI
      {{0, 0, 0, 0}, {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},   // kFigureL
      {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   // kFigureO
      {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},   // kFigureT
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},   // kFigureS
      {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},   // kFigureZ
      {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}}};  // kFigureJ
  copyTetromino(ptr_fig->cell, tetrominoes[type]);
  ptr_fig->type = type;
}

void generateNextFigure() {
  TetrisInfo_t *game = getTetrisInfo();
  Tetromino_t type = rand() % 7;
  // Generate next tetromino if empty (start of game)
  static bool next_empty = true;
  if (next_empty) {
    setFigure(&game->next.fig, type);
    type = rand() % 7;
    next_empty = false;
  }
  setFigure(&game->current.fig, game->next.fig.type);
  game->current.coordinate.x = 3;
  game->current.coordinate.y = (game->current.fig.type == kFigureI ? -2 : -3);
  setFigure(&game->next.fig, type);
}

void onStartState(UserAction_t action) {
  switch (action) {
    case Start:
      generateNextFigure();
      setState(kMoving);
      break;
    case Terminate:
      handleTerminateState();
      break;
    default:
      break;
  }
}

void onPauseState(UserAction_t action) {
  TetrisInfo_t *game = getTetrisInfo();
  switch (action) {
    case Pause:
      setState(kMoving);
      game->pause = 0;
      break;
    case Terminate:
      handleTerminateState();
      break;
    default:
      break;
  }
}

int getLowestCoordinate() {
  TetrisInfo_t *game = getTetrisInfo();
  int lowest_row = 0;
  for (int i = kFigRows - 1; i > 0 && lowest_row == 0; i--) {
    for (int j = 0; j < kFigCols && lowest_row == 0; j++) {
      if (game->current.fig.cell[i][j]) {
        lowest_row = i;
      }
    }
  }
  return game->current.coordinate.y + lowest_row;
}

bool checkGameOver() {
  bool game_over = false;
  // If figure was attched in row 0
  if (getLowestCoordinate() <= 0) {
    game_over = true;
  }
  return game_over;
}

bool isLineFill(int line) {
  TetrisInfo_t *game = getTetrisInfo();
  bool line_is_fill = true;
  for (int j = 0; j < kCols && line_is_fill; j++) {
    if (game->field.cell[line][j] == false) {
      line_is_fill = false;
    }
  }
  return line_is_fill;
}

void moveGroundDown(int line) {
  TetrisInfo_t *game = getTetrisInfo();
  for (int i = line; i > 0; i--) {
    for (int j = 0; j < kCols; j++) {
      game->field.cell[i][j] = game->field.cell[i - 1][j];
    }
  }
  for (int j = 0; j < kCols; j++) {
    game->field.cell[0][j] = 0;
  }
}

void handleAttaching() {
  TetrisInfo_t *game = getTetrisInfo();
  int count_filled_lines = 0;
  for (int line = 0; line < kRows; line++) {
    if (isLineFill(line)) {
      count_filled_lines += 1;
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
#ifndef NO_LIMITS
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
#endif  // NO_LIMITS
#ifdef NO_LIMITS
  if (count_filled_lines > 0) {
    game->level = game->score / 600;
    game->speed = game->level;
    game->update_interval = 1000 - game->speed * 75;
  }
#endif  // NO_LIMITS
  game->current.hash_all_rotation = 0;
  game->current.rotation = 0;
}

bool checkNewPosition() {
  TetrisInfo_t *game = getTetrisInfo();
  bool can_move = true;
  for (int i = 0; i < kFigRows && can_move; i++) {
    for (int j = 0; j < kFigCols && can_move; j++) {
      if (game->current.fig.cell[i][j]) {
        int new_x = game->current.coordinate.x + game->current.offset_x + j;
        int new_y = game->current.coordinate.y + game->current.offset_y + i;
        if (figureCannotMove(new_x, new_y, game->field.cell[new_y][new_x])) {
          can_move = false;
        }
      }
    }
  }
  return can_move;
}

void addFigureOnField() {
  TetrisInfo_t *game = getTetrisInfo();
  for (int i = 0; i < kFigRows; i++) {
    for (int j = 0; j < kFigCols; j++) {
      if (game->current.fig.cell[i][j]) {
        int new_x = game->current.coordinate.x + game->current.offset_x + j;
        int new_y = game->current.coordinate.y + game->current.offset_y + i;
        if (coordinateInField(new_x, new_y)) {
          game->field.cell[new_y][new_x] = game->current.fig.cell[i][j];
        }
      }
    }
  }
}

bool tryMoveFigure(UserAction_t action) {
  TetrisInfo_t *game = getTetrisInfo();

  game->current.offset_x -= (action == Left);
  game->current.offset_x += (action == Right);
  game->current.offset_y = (action == Down);

  eraseCurrentFigureOnField();

  bool can_move = checkNewPosition();

  if (can_move) {
    addFigureOnField();
    game->current.coordinate.x += game->current.offset_x;
    game->current.coordinate.y += game->current.offset_y;
    game->current.offset_x = 0;
    game->current.offset_y = 0;
  } else {
    game->current.offset_x = 0;
    game->current.offset_y = 0;
    addFigureOnField();
  }
  return can_move;
}

void rotateCurrentFigure() {
  TetrisInfo_t *game = getTetrisInfo();
  game->current.rotation = game->current.hash_all_rotation % 4;
  switch (game->current.fig.type) {
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
  for (int i = 0; i < kFigRows; i++) {
    for (int j = 0; j < kFigCols; j++) {
      if (game->current.fig.cell[i][j]) {
        int fx = game->current.coordinate.x + j;
        int fy = game->current.coordinate.y + i;
        if (coordinateInField(fx, fy)) {
          game->field.cell[fy][fx] = 0;
        }
      }
    }
  }
}

void onGameOverState(UserAction_t action) {
  switch (action) {
    case Start:
      clearTetrisInfo();
      generateNextFigure();
      setState(kMoving);
      break;
    case Terminate:
      handleTerminateState();
      break;
    default:
      break;
  }
}

void dropFigure() {
  while (tryMoveFigure(Down) == true) {
  }
}

bool tryRotateFigure() {
  TetrisInfo_t *game = getTetrisInfo();

  eraseCurrentFigureOnField();

  game->current.hash_all_rotation += 1;
  rotateCurrentFigure();

  bool can_move = checkNewPosition();

  if (can_move) {
    addFigureOnField();
  } else {
    // Turn back last position
    game->current.hash_all_rotation -= 1;
    rotateCurrentFigure();
    addFigureOnField();
  }
  return can_move;
}

void clearCurrentFigure() {
  TetrisInfo_t *game = getTetrisInfo();
  game->current.fig.cell[0][0] = 0;
  game->current.fig.cell[0][1] = 0;
  game->current.fig.cell[0][2] = 0;
  game->current.fig.cell[0][3] = 0;
  game->current.fig.cell[1][0] = 0;
  game->current.fig.cell[1][1] = 0;
  game->current.fig.cell[1][2] = 0;
  game->current.fig.cell[1][3] = 0;
  game->current.fig.cell[2][0] = 0;
  game->current.fig.cell[2][1] = 0;
  game->current.fig.cell[2][2] = 0;
  game->current.fig.cell[2][3] = 0;
  game->current.fig.cell[3][0] = 0;
  game->current.fig.cell[3][1] = 0;
  game->current.fig.cell[3][2] = 0;
  game->current.fig.cell[3][3] = 0;
}

void rotateFigureI() {
  TetrisInfo_t *game = getTetrisInfo();
  clearCurrentFigure();
  switch (game->current.rotation) {
    case 1:
      game->current.fig.cell[0][2] = 1;  //  . .[] .
      game->current.fig.cell[1][2] = 1;  //  . .[] .
      game->current.fig.cell[2][2] = 1;  //  . .[] .
      game->current.fig.cell[3][2] = 1;  //  . .[] .
      break;
    case 2:
      game->current.fig.cell[2][0] = 1;  //  . . . .
      game->current.fig.cell[2][1] = 1;  //  . . . .
      game->current.fig.cell[2][2] = 1;  // [][][][]
      game->current.fig.cell[2][3] = 1;  //  . . . .
      break;
    case 3:
      game->current.fig.cell[0][1] = 1;  //  .[] . .
      game->current.fig.cell[1][1] = 1;  //  .[] . .
      game->current.fig.cell[2][1] = 1;  //  .[] . .
      game->current.fig.cell[3][1] = 1;  //  .[] . .
      break;
    case 0:
      game->current.fig.cell[1][0] = 1;  //  . . . .
      game->current.fig.cell[1][1] = 1;  // [][][][]
      game->current.fig.cell[1][2] = 1;  //  . . . .
      game->current.fig.cell[1][3] = 1;  //  . . . .
      break;
  }
}
void rotateFigureJ() {
  TetrisInfo_t *game = getTetrisInfo();
  clearCurrentFigure();
  switch (game->current.rotation) {
    case 1:
      game->current.fig.cell[0][1] = 1;  //  .[] . .
      game->current.fig.cell[1][1] = 1;  //  .[] . .
      game->current.fig.cell[2][0] = 1;  // [][] . .
      game->current.fig.cell[2][1] = 1;  //  . . . .
      break;
    case 2:
      game->current.fig.cell[0][0] = 1;  // [] . . .
      game->current.fig.cell[1][0] = 1;  // [][][] .
      game->current.fig.cell[1][1] = 1;  //  . . . .
      game->current.fig.cell[1][2] = 1;  //  . . . .
      break;
    case 3:
      game->current.fig.cell[0][1] = 1;  //  .[][] .
      game->current.fig.cell[0][2] = 1;  //  .[] . .
      game->current.fig.cell[1][1] = 1;  //  .[] . .
      game->current.fig.cell[2][1] = 1;  //  . . . .
      break;
    case 0:
      game->current.fig.cell[1][0] = 1;  //  . . . .
      game->current.fig.cell[1][1] = 1;  // [][][] .
      game->current.fig.cell[1][2] = 1;  //  . .[] .
      game->current.fig.cell[2][2] = 1;  //  . . . .
      break;
  }
}
void rotateFigureT() {
  TetrisInfo_t *game = getTetrisInfo();
  clearCurrentFigure();
  switch (game->current.rotation) {
    case 1:
      game->current.fig.cell[0][1] = 1;  //  .[] . .
      game->current.fig.cell[1][0] = 1;  // [][] . .
      game->current.fig.cell[1][1] = 1;  //  .[] . .
      game->current.fig.cell[2][1] = 1;  //  . . . .
      break;
    case 2:
      game->current.fig.cell[0][1] = 1;  //  .[] . .
      game->current.fig.cell[1][0] = 1;  // [][][] .
      game->current.fig.cell[1][1] = 1;  //  . . . .
      game->current.fig.cell[1][2] = 1;  //  . . . .
      break;
    case 3:
      game->current.fig.cell[0][1] = 1;  //  .[] . .
      game->current.fig.cell[1][1] = 1;  //  .[][] .
      game->current.fig.cell[1][2] = 1;  //  .[] . .
      game->current.fig.cell[2][1] = 1;  //  . . . .
      break;
    case 0:
      game->current.fig.cell[1][0] = 1;  //  . . . .
      game->current.fig.cell[1][1] = 1;  // [][][] .
      game->current.fig.cell[1][2] = 1;  //  .[] . .
      game->current.fig.cell[2][1] = 1;  //  . . . .
      break;
  }
}
void rotateFigureS() {
  TetrisInfo_t *game = getTetrisInfo();
  clearCurrentFigure();
  switch (game->current.rotation) {
    case 1:
      game->current.fig.cell[0][0] = 1;  // [] . . .
      game->current.fig.cell[1][0] = 1;  // [][] . .
      game->current.fig.cell[1][1] = 1;  //  .[] . .
      game->current.fig.cell[2][1] = 1;  //  . . . .
      break;
    case 2:
      game->current.fig.cell[0][1] = 1;  //  .[][] .
      game->current.fig.cell[0][2] = 1;  // [][] . .
      game->current.fig.cell[1][0] = 1;  //  . . . .
      game->current.fig.cell[1][1] = 1;  //  . . . .
      break;
    case 3:
      game->current.fig.cell[0][1] = 1;  //  .[] . .
      game->current.fig.cell[1][1] = 1;  //  .[][] .
      game->current.fig.cell[1][2] = 1;  //  . .[] .
      game->current.fig.cell[2][2] = 1;  //  . . . .
      break;
    case 0:
      game->current.fig.cell[1][1] = 1;  //  . . . .
      game->current.fig.cell[1][2] = 1;  //  .[][] .
      game->current.fig.cell[2][0] = 1;  // [][] . .
      game->current.fig.cell[2][1] = 1;  //  . . . .
      break;
  }
}
void rotateFigureZ() {
  TetrisInfo_t *game = getTetrisInfo();
  clearCurrentFigure();
  switch (game->current.rotation) {
    case 1:
      game->current.fig.cell[0][1] = 1;  //  .[] . .
      game->current.fig.cell[1][0] = 1;  // [][] . .
      game->current.fig.cell[1][1] = 1;  // [] . . .
      game->current.fig.cell[2][0] = 1;  //  . . . .
      break;
    case 2:
      game->current.fig.cell[0][0] = 1;  // [][] . .
      game->current.fig.cell[0][1] = 1;  //  .[][] .
      game->current.fig.cell[1][1] = 1;  //  . . . .
      game->current.fig.cell[1][2] = 1;  //  . . . .
      break;
    case 3:
      game->current.fig.cell[0][2] = 1;  //  . .[] .
      game->current.fig.cell[1][1] = 1;  //  .[][] .
      game->current.fig.cell[1][2] = 1;  //  .[] . .
      game->current.fig.cell[2][1] = 1;  //  . . . .
      break;
    case 0:
      game->current.fig.cell[1][0] = 1;  //  . . . .
      game->current.fig.cell[1][1] = 1;  // [][] . .
      game->current.fig.cell[2][1] = 1;  //  .[][] .
      game->current.fig.cell[2][2] = 1;  //  . . . .
      break;
  }
}
void rotateFigureL() {
  TetrisInfo_t *game = getTetrisInfo();
  clearCurrentFigure();
  switch (game->current.rotation) {
    case 1:
      game->current.fig.cell[0][0] = 1;  // [][] . .
      game->current.fig.cell[0][1] = 1;  //  .[] . .
      game->current.fig.cell[1][1] = 1;  //  .[] . .
      game->current.fig.cell[2][1] = 1;  //  . . . .
      break;
    case 2:
      game->current.fig.cell[0][2] = 1;  //  . .[] .
      game->current.fig.cell[1][0] = 1;  // [][][] .
      game->current.fig.cell[1][1] = 1;  //  . . . .
      game->current.fig.cell[1][2] = 1;  //  . . . .
      break;
    case 3:
      game->current.fig.cell[0][1] = 1;  //  .[] . .
      game->current.fig.cell[1][1] = 1;  //  .[] . .
      game->current.fig.cell[2][1] = 1;  //  .[][] .
      game->current.fig.cell[2][2] = 1;  //  . . . .
      break;
    case 0:
      game->current.fig.cell[1][0] = 1;  //  . . . .
      game->current.fig.cell[1][1] = 1;  // [][][] .
      game->current.fig.cell[1][2] = 1;  // [] . . .
      game->current.fig.cell[2][0] = 1;  //  . . . .
      break;
  }
}
