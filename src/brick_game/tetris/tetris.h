#ifndef BRICK_GAME_TETRIS_H_
#define BRICK_GAME_TETRIS_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

/** States of FSM */
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
  int x;
  int y;
} Point_t;

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
    int *row[FIG_ROWS];
    int cell[FIG_ROWS][FIG_COLS];
    int offset_x;
    int offset_y;
    int rotation;
    int hash_all_rotation;
  } curr_fig;

  bool run_game;
  int level;
  int speed;
  int score;
  int high_score;
  unsigned long last_tick;        // time
  unsigned long update_interval;  // time
} TetrisInfo_t;

TetrisState_t *initState();
TetrisState_t *getState();
void setState(TetrisState_t new_state);
GameInfo_t *getGameInfo();
TetrisInfo_t *initTetrisInfo();
TetrisInfo_t *getTetrisInfo();
void clearTetrisInfo();
void clearArray(int **array, int rows, int cols);

void userInput(UserAction_t action, bool hold);
GameInfo_t updateCurrentState();

unsigned long currentTimeMs();
bool timeToShift();
void saveHighScore();
bool coordinateInField(const int x, const int y);
bool figureCannotMove(const int x, const int y, const int field_cell);
void copyTetromino(int dst_fig[FIG_ROWS][FIG_COLS],
                   int src_fig[FIG_ROWS][FIG_COLS]);
void onStartState(UserAction_t action);
void onPauseState(UserAction_t action);
void onGameOverState(UserAction_t action);
void onMovingState(UserAction_t action);
void handleSpawnState();
void handleAttachingState();
void handleTerminateState();
int getLowestCoordinate();
bool checkGameOver();
bool isLineFill(int line);
void moveGroundDown(int line);
bool tryMoveFigure(UserAction_t action);
void rotateCurrentFigure();
void eraseCurrentFigureOnField();
void dropFigure();
bool tryRotateFigure();
void rotateFigureI();
void rotateFigureJ();
void rotateFigureT();
void rotateFigureS();
void rotateFigureZ();
void rotateFigureL();

#endif  // BRICK_GAME_TETRIS_H_
