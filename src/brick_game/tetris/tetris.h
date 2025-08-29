#ifndef BRICK_GAME_TETRIS_H_
#define BRICK_GAME_TETRIS_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../brick_game.h"

/** States of FSM */
typedef enum {
  kStart,
  kPause,
  kMoving,
  kGameOver
} TetrisState_t;

typedef enum {
  kFigCols = 4,
  kFigRows = 4,
  kCols = 10,
  kRows = 20
} Sizes_t;

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
  Tetromino_t type;
  int *row[kFigRows];
  int cell[kFigRows][kFigCols];
} Figure_t;

typedef struct {
  struct {
    int *row[kRows];
    int cell[kRows][kCols];
  } field;

  struct {
    Figure_t fig;
  } next;

  struct {
    Figure_t fig;
    Point_t coordinate;
    int offset_x;
    int offset_y;
    int rotation;
    int hash_all_rotation;
  } current;

  bool run_game;
  int level;
  int speed;
  int score;
  int high_score;
  int pause;
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
void clearArray(int **array, int kRows, int kCols);

unsigned long currentTimeMs();
bool timeToShift();
void saveHighScore();
bool coordinateInField(const int x, const int y);
bool figureCannotMove(const int x, const int y, const int field_cell);
void copyTetromino(int dst_fig[kFigRows][kFigCols],
                   int src_fig[kFigRows][kFigCols]);
void onStartState(UserAction_t action);
void onPauseState(UserAction_t action);
void onGameOverState(UserAction_t action);
void onMovingState(UserAction_t action);
void generateNextFigure();
void handleAttaching();
void handleTerminateState();
int getLowestCoordinate();
bool checkGameOver();
bool isLineFill(int line);
void moveGroundDown(int line);
bool tryMoveFigure(UserAction_t action);
bool checkNewPosition();
void addFigureOnField();
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
void clearCurrentFigure();

#endif  // BRICK_GAME_TETRIS_H_
