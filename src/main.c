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
    keypad(stdscr, true); \
    noecho();             \
    curs_set(0);          \
    timeout(1);           \
  }
#define UPDATE_INTERVAL_MS 100

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
    int offset_x;
    int offset_y;
  } curr_fig;
  UserAction_t action;
  bool hold;
  int collapsing_rows;
} TetrisInfo_t;

typedef enum {
  kStart,
  kPause,
  kTerminate,
  kSpawn,
  kMovingLeft,
  kMovingRight,
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

void onStartState(UserAction_t action);
void onPauseState(UserAction_t action);
void onShiftingState(UserAction_t action);
void onGameOverState(UserAction_t action);

void handleSpawnState();
void handleShiftingState();
void handleAttachingState();

bool figureCanMove(const int fx, const int fy, const int field_cell);

void debugWhichState(TetrisState_t *ptr_state, char *buffer);

bool coordinateInField(const int x, const int y);
bool figureCannotMove(const int x, const int y, const int field_cell);

void moveFigureLeft();
void moveFigureRight();

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

void handleShiftingState() {
  TetrisInfo_t *ptr = getTetrisInfo();

  int x = ptr->curr_fig.coordinate.x;
  int y = ptr->curr_fig.coordinate.y;

  int offset_x = ptr->curr_fig.offset_x = 0;
  int offset_y = ptr->curr_fig.offset_y = 1;

  // if (y < 0) {  // Есть ощущение, что потребуется это условие
  // Когда часть фигуры может наезжать на уже заполненные блоки верхнего ряда
  // и тем самым стереть их

  // Erase current position
  for (int i = 0; i < FIG_ROWS; i++) {
    for (int j = 0; j < FIG_COLS; j++) {
      if (ptr->curr_fig.cell[i][j]) {
        int fx = x + j;
        int fy = y + i;
        if (coordinateInField(fx, fy)) {
          ptr->field.cell[fy][fx] = 0;
        }
      }
    }
  }

  // }  // закрытие if (y < 0)

  // Check new position
  bool can_move = true;
  for (int i = 0; i < FIG_ROWS && can_move; i++) {
    for (int j = 0; j < FIG_COLS && can_move; j++) {
      if (ptr->curr_fig.cell[i][j]) {
        int new_x = x + j + offset_x;
        int new_y = y + i + offset_y;
        if (figureCannotMove(new_x, new_y, ptr->field.cell[new_y][new_x])) {
          can_move = false;
        }
      }
    }
  }

  if (can_move) {
    // Add figure to new position
    for (int i = 0; i < FIG_ROWS; i++) {
      for (int j = 0; j < FIG_COLS; j++) {
        if (ptr->curr_fig.cell[i][j]) {
          int fx = x + j + offset_x;
          int fy = y + i + offset_y;
          if (coordinateInField(fx, fy)) {
            ptr->field.cell[fy][fx] = ptr->curr_fig.cell[i][j];
          }
        }
      }
    }
    ptr->curr_fig.coordinate.y++;
  } else {
    // Turn back last position
    for (int i = 0; i < FIG_ROWS; i++) {
      for (int j = 0; j < FIG_COLS; j++) {
        if (ptr->curr_fig.cell[i][j]) {
          int fx = x + j;
          int fy = y + i;
          if (coordinateInField(fx, fy)) {
            ptr->field.cell[fy][fx] = ptr->curr_fig.cell[i][j];
          }
        }
      }
    }
    setState(kAttaching);  // Корректировать: исключительно когда фигура падает
                           // на что-то
    // Возможно стоит изменить логику kShifting. Сделать это состояние Moving
    // А kShifting устанавливается по истечению таймера.
    // Если время вышло, а снизу есть место, то kShifting.
    // если нет места, то kAttaching
  }
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
    // hardcode for testing specific figure
    // copyTetromino(ptr_tetris_info->next.cell, tetrominos[2]);
    next_empty = false;
  }
  copyTetromino(ptr_tetris_info->curr_fig.cell, ptr_tetris_info->next.cell);

  // Set starting coordinate (top center)
  ptr_tetris_info->curr_fig.coordinate.x = 3;
  ptr_tetris_info->curr_fig.coordinate.y = -2;

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
void onPauseState(UserAction_t action) {
  switch (action) {
    case Start:
      setState(kShifting);
      break;
    case Terminate:
      setState(kTerminate);
      break;
  }
}

void handleAttachingState() {
  TetrisInfo_t *ptr_tetris_info = getTetrisInfo();
  // If figure was attched in row 0
  if (ptr_tetris_info->curr_fig.coordinate.y == -2) {
    // Game over
    setState(kGameOver);
  }
  // Else
  // Check filled lines
  // Shift the top pixels to their place
  // Earn points
  // Set new level necessary
  // Set new speed necessary

  setState(kSpawn);
}

void onGameOverState(UserAction_t action) {
  switch (action) {
    case Start:
      // Clear TetrisInfo
      setState(kSpawn);
      break;
    case Terminate:
      setState(kTerminate);
      break;
  }
}

void onShiftingState(UserAction_t action) {
  switch (action) {
    case Left:
      setState(kMovingLeft);
      break;
    case Right:
      setState(kMovingRight);
      break;
    case Terminate:
      setState(kTerminate);
      break;
    case Pause:
      setState(kPause);
      break;
      // case :
      //   setState( );
      //   break;

      // Attaching по таймеру ???
  }
}

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

void moveFigureLeft() {
  // Нужно менять логику расположения на поле
    TetrisInfo_t *ptr = getTetrisInfo();

  int x = ptr->curr_fig.coordinate.x;
  int y = ptr->curr_fig.coordinate.y;

  int offset_x = ptr->curr_fig.offset_x = -1;
  int offset_y = ptr->curr_fig.offset_y = 0;

  // Erase current position
  for (int i = 0; i < FIG_ROWS; i++) {
    for (int j = 0; j < FIG_COLS; j++) {
      if (ptr->curr_fig.cell[i][j]) {
        int fx = x + j;
        int fy = y + i;
        if (coordinateInField(fx, fy)) {
          ptr->field.cell[fy][fx] = 0;
        }
      }
    }
  }

  // Check new position
  bool can_move = true;
  for (int i = 0; i < FIG_ROWS && can_move; i++) {
    for (int j = 0; j < FIG_COLS && can_move; j++) {
      if (ptr->curr_fig.cell[i][j]) {
        int new_x = x + j + offset_x;
        int new_y = y + i + offset_y;
        if (figureCannotMove(new_x, new_y, ptr->field.cell[new_y][new_x])) {
          can_move = false;
        }
      }
    }
  }

  if (can_move) {
    // Add figure to new position
    for (int i = 0; i < FIG_ROWS; i++) {
      for (int j = 0; j < FIG_COLS; j++) {
        if (ptr->curr_fig.cell[i][j]) {
          int fx = x + j + offset_x;
          int fy = y + i + offset_y;
          if (coordinateInField(fx, fy)) {
            ptr->field.cell[fy][fx] = ptr->curr_fig.cell[i][j];
          }
        }
      }
    }
    ptr->curr_fig.coordinate.x--;
  } else {
    // Turn back last position
    for (int i = 0; i < FIG_ROWS; i++) {
      for (int j = 0; j < FIG_COLS; j++) {
        if (ptr->curr_fig.cell[i][j]) {
          int fx = x + j;
          int fy = y + i;
          if (coordinateInField(fx, fy)) {
            ptr->field.cell[fy][fx] = ptr->curr_fig.cell[i][j];
          }
        }
      }
    }
  }
  setState(kShifting);
}

void moveFigureRight() {
    // Нужно менять логику расположения на поле
    TetrisInfo_t *ptr = getTetrisInfo();

  int x = ptr->curr_fig.coordinate.x;
  int y = ptr->curr_fig.coordinate.y;

  int offset_x = ptr->curr_fig.offset_x = 1;
  int offset_y = ptr->curr_fig.offset_y = 0;

  // Erase current position
  for (int i = 0; i < FIG_ROWS; i++) {
    for (int j = 0; j < FIG_COLS; j++) {
      if (ptr->curr_fig.cell[i][j]) {
        int fx = x + j;
        int fy = y + i;
        if (coordinateInField(fx, fy)) {
          ptr->field.cell[fy][fx] = 0;
        }
      }
    }
  }

  // Check new position
  bool can_move = true;
  for (int i = 0; i < FIG_ROWS && can_move; i++) {
    for (int j = 0; j < FIG_COLS && can_move; j++) {
      if (ptr->curr_fig.cell[i][j]) {
        int new_x = x + j + offset_x;
        int new_y = y + i + offset_y;
        if (figureCannotMove(new_x, new_y, ptr->field.cell[new_y][new_x])) {
          can_move = false;
        }
      }
    }
  }

  if (can_move) {
    // Add figure to new position
    for (int i = 0; i < FIG_ROWS; i++) {
      for (int j = 0; j < FIG_COLS; j++) {
        if (ptr->curr_fig.cell[i][j]) {
          int fx = x + j + offset_x;
          int fy = y + i + offset_y;
          if (coordinateInField(fx, fy)) {
            ptr->field.cell[fy][fx] = ptr->curr_fig.cell[i][j];
          }
        }
      }
    }
    ptr->curr_fig.coordinate.x++;
  } else {
    // Turn back last position
    for (int i = 0; i < FIG_ROWS; i++) {
      for (int j = 0; j < FIG_COLS; j++) {
        if (ptr->curr_fig.cell[i][j]) {
          int fx = x + j;
          int fy = y + i;
          if (coordinateInField(fx, fy)) {
            ptr->field.cell[fy][fx] = ptr->curr_fig.cell[i][j];
          }
        }
      }
    }
  }
  setState(kShifting);
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
    case KEY_Q_LOWER:  // Quit from the BrickGame
      action = Terminate;
      break;
  }
  return action;
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
    case kShifting:
      onShiftingState(action);
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
      handleShiftingState();
      break;
    case kAttaching:
      handleAttachingState();
      break;
    case kMovingLeft:
      moveFigureLeft();
      break;
    case kMovingRight:
      moveFigureRight();
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
    case kMovingLeft:
      strcpy(buffer, "kMovingLeft");
      break;
    case kMovingRight:
      strcpy(buffer, "kMovingRight");
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

long long current_time_ms() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long long)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// BrickGame function
void gameCycle() {
  UserAction_t action = Up;
  bool hold;
  GameInfo_t info;
  TetrisState_t *ptr_state = getState();
  long long last_update = current_time_ms();
  bool run_game = true;
  while (run_game) {
    if (*ptr_state == kTerminate) {
      run_game = false;
    }

    userInput(action, hold);
    long long now = current_time_ms();
    if (now - last_update >= UPDATE_INTERVAL_MS) {
      info = updateCurrentState();
      showState(info);
      last_update = now;
    }
    if (*ptr_state == kShifting || *ptr_state == kStart ||
        *ptr_state == kGameOver || *ptr_state == kPause) {
      action = getSignal();
    }
    usleep(10000);
  }
}

int main() {
  srand(time(NULL));
  WIN_INIT();
  // run BrickGame
  // Should BrickGame let choose the game?
  // The game was choosed
  gameCycle();
  endwin();
  return 0;
}