#include "tetris.h"

#include <check.h>

#include "../../gui/cli/cli.h"
#include "../brick_game.h"

#ifdef PRINT_TEST
void printArray(int **array, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      printf("%d ", array[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}
#endif  // PRINT_TEST

// FSM    -> kStart
// action -> Start
START_TEST(onStartStateGetStart) {
  // Arrange
  setState(kStart);
  TetrisInfo_t *game = getTetrisInfo();
  GameInfo_t game_info = {0};
  // Act
  userInput(Start, false);
  game_info = updateCurrentState();
  // Assert
  ck_assert_int_eq(game->run_game, true);
  ck_assert_ptr_nonnull(game_info.field);
  ck_assert_ptr_nonnull(game_info.next);
}
END_TEST

// FSM    -> kStart
// action -> Terminate
START_TEST(onStartStateGetTerminate) {
  // Arrange
  setState(kStart);
  TetrisInfo_t *game = getTetrisInfo();
  GameInfo_t game_info = {0};
  // Act
  userInput(Terminate, false);
  updateCurrentState();
  // Assert
  ck_assert_int_eq(game->run_game, false);
  ck_assert_ptr_null(game_info.field);
  ck_assert_ptr_null(game_info.next);
}
END_TEST

// FSM    -> kMoving
// action -> Left
START_TEST(onMovingStateGetLeft) {
  // Arrange
  // . . . . . . . . . .
  // . . . .[][] . . . .
  // . . . .[][] . . . .
  // . . . . . . . . . .
  //        ...
  setState(kMoving);
  TetrisInfo_t *game = getTetrisInfo();
  userInput(Start, false);
  setFigure(&game->current.fig, kFigureO);
  game->current.coordinate.x = 3;
  game->current.coordinate.y = 0;
  tryMoveFigure(Down);
  GameInfo_t game_info = *getGameInfo();

#ifdef PRINT_TEST
  printArray(game_info.field, kRows, kCols);
#endif

  // Act
  userInput(Left, false);
  game_info = updateCurrentState();
  // Assert
  // . . . . . . . . . .
  // . . .[][] . . . . .
  // . . .[][] . . . . .
  // . . . . . . . . . .
  //        ...

#ifdef PRINT_TEST
  printArray(game_info.field, kRows, kCols);
#endif

  ck_assert_int_eq(game_info.field[1][3], 1);  // new pos
  ck_assert_int_eq(game_info.field[1][4], 1);  // the same
  ck_assert_int_eq(game_info.field[1][5], 0);  // old pos

  ck_assert_int_eq(game_info.field[2][3], 1);  // new pos
  ck_assert_int_eq(game_info.field[2][4], 1);  // the same
  ck_assert_int_eq(game_info.field[2][5], 0);  // old pos
}
END_TEST

// FSM    -> kMoving
// action -> Right
START_TEST(onMovingStateGetRight) {
  // Arrange
  // . . . . . . . . . .
  // . . . .[][] . . . .
  // . . . .[][] . . . .
  // . . . . . . . . . .
  //        ...
  setState(kMoving);
  TetrisInfo_t *game = getTetrisInfo();
  userInput(Start, false);
  setFigure(&game->current.fig, kFigureO);
  game->current.coordinate.x = 3;
  game->current.coordinate.y = 0;
  tryMoveFigure(Down);
  GameInfo_t game_info = *getGameInfo();

#ifdef PRINT_TEST
  printArray(game_info.field, kRows, kCols);
#endif

  // Act
  userInput(Right, false);
  game_info = updateCurrentState();
  // Assert
  // . . . . . . . . . .
  // . . . . .[][] . . .
  // . . . . .[][] . . .
  // . . . . . . . . . .
  //        ...

#ifdef PRINT_TEST
  printArray(game_info.field, kRows, kCols);
#endif

  ck_assert_int_eq(game_info.field[1][4], 0);  // old pos
  ck_assert_int_eq(game_info.field[1][5], 1);  // the same
  ck_assert_int_eq(game_info.field[1][6], 1);  // new pos

  ck_assert_int_eq(game_info.field[2][4], 0);  // old pos
  ck_assert_int_eq(game_info.field[2][5], 1);  // the same
  ck_assert_int_eq(game_info.field[2][6], 1);  // new pos
}
END_TEST

// FSM    -> kMoving
// action -> Down
START_TEST(onMovingStateGetDown) {
  // Arrange
  // . . . . . . . . . .
  // . . . .[][] . . . .
  // . . . .[][] . . . .
  // . . . . . . . . . .
  //        ...
  setState(kMoving);
  TetrisInfo_t *game = getTetrisInfo();
  userInput(Start, false);
  setFigure(&game->current.fig, kFigureO);
  game->current.coordinate.x = 3;
  game->current.coordinate.y = 0;
  tryMoveFigure(Down);
  GameInfo_t game_info = *getGameInfo();

#ifdef PRINT_TEST
  printArray(game_info.field, kRows, kCols);
#endif

  // Act
  userInput(Down, false);
  game_info = updateCurrentState();
  // Assert
  //        ...
  // . . . . . . . . . .
  // . . . .[][] . . . .
  // . . . .[][] . . . .

#ifdef PRINT_TEST
  printArray(game_info.field, kRows, kCols);
#endif

  ck_assert_int_eq(game_info.field[1][4], 0);  // old pos
  ck_assert_int_eq(game_info.field[1][5], 0);  // old pos
  ck_assert_int_eq(game_info.field[2][4], 0);  // old pos
  ck_assert_int_eq(game_info.field[2][5], 0);  // old pos

  ck_assert_int_eq(game_info.field[18][4], 1);  // new pos
  ck_assert_int_eq(game_info.field[18][5], 1);  // new pos
  ck_assert_int_eq(game_info.field[19][4], 1);  // new pos
  ck_assert_int_eq(game_info.field[19][5], 1);  // new pos
}
END_TEST

// FSM      -> kMoving
// action   -> Action
// figure   -> kFigureT
// rotation -> 1
START_TEST(onMovingStateGetActionFigureTRotation1) {
  // Arrange
  // . . . . . . . . . .
  // . . . .[][][] . . .
  // . . . . .[] . . . .
  // . . . . . . . . . .
  //        ...
  setState(kMoving);
  TetrisInfo_t *game = getTetrisInfo();
  userInput(Start, false);
  setFigure(&game->current.fig, kFigureT);
  game->current.coordinate.x = 4;
  game->current.coordinate.y = -1;
  tryMoveFigure(Down);
  GameInfo_t game_info = *getGameInfo();

#ifdef PRINT_TEST
  printArray(game_info.field, kRows, kCols);
#endif

  // Act
  userInput(Action, false);
  game_info = updateCurrentState();
  // Assert
  // . . . . .[] . . . .
  // . . . .[][] . . . .
  // . . . . .[] . . . .
  // . . . . . . . . . .
  //        ...

#ifdef PRINT_TEST
  printArray(game_info.field, kRows, kCols);
#endif

  ck_assert_int_eq(game_info.field[0][5], 1);  // new pos
  ck_assert_int_eq(game_info.field[1][4], 1);  // the same
  ck_assert_int_eq(game_info.field[1][5], 1);  // the same
  ck_assert_int_eq(game_info.field[1][6], 0);  // old pos
  ck_assert_int_eq(game_info.field[2][5], 1);  // the same

}
END_TEST

// FSM    -> kMoving
// action -> Pause

// FSM    -> kMoving
// action -> Terminate

// FSM    -> kPause
// action -> Pause

// FSM    -> kPause
// action -> Terminate

// FSM    -> kGameOver
// action -> Start
START_TEST(onGameOverStateGetStart) {
  // Arrange
  setState(kGameOver);
  TetrisInfo_t *game = getTetrisInfo();
  GameInfo_t game_info = {0}; // положить что-нибудь?

  // Act
  userInput(Start, false);
  game_info = updateCurrentState();
  // Assert
  ck_assert_int_eq(game->run_game, true);
  ck_assert_ptr_nonnull(game_info.field);
  ck_assert_ptr_nonnull(game_info.next);
  ck_assert_int_eq(game_info.level, 0);
  ck_assert_int_eq(game_info.score, 0);
  ck_assert_int_eq(game_info.speed, 0);
}
END_TEST

// FSM    -> kGameOver
// action -> Terminate
START_TEST(onGameOverStateGetTerminate) {
  // Arrange
  setState(kGameOver);
  TetrisInfo_t *game = getTetrisInfo();
  GameInfo_t game_info = {0};
  // Act
  userInput(Terminate, false);
  game_info = updateCurrentState();
  // Assert
  ck_assert_int_eq(game->run_game, false);
  ck_assert_ptr_null(game_info.field);
  ck_assert_ptr_null(game_info.next);
}
END_TEST

Suite *create_suite_tetris(void) {
  Suite *suite = suite_create("Suite of Tetris");
  TCase *tc_core = tcase_create("Test cases of Tetris");

  // Start state tests
  tcase_add_test(tc_core, onStartStateGetStart);
  tcase_add_test(tc_core, onStartStateGetTerminate);

  // Moving state tests
  tcase_add_test(tc_core, onMovingStateGetLeft);
  tcase_add_test(tc_core, onMovingStateGetRight);
  tcase_add_test(tc_core, onMovingStateGetDown);
  tcase_add_test(tc_core, onMovingStateGetActionFigureTRotation1);

  // Pause state tests

  // GameOver state tests
  tcase_add_test(tc_core, onGameOverStateGetStart);
  tcase_add_test(tc_core, onGameOverStateGetTerminate);

  suite_add_tcase(suite, tc_core);

  return suite;
}

int main(void) {
  // Установлен большой интервал обновления, чтобы
  // исключить сдвиг фигур по таймеру
  // мешающий при проверке перемещений
  TetrisInfo_t *game = getTetrisInfo();
  game->update_interval = 9999999999UL;

  int failed_counter, exit_status;
  Suite *suite = create_suite_tetris();
  SRunner *suite_runner = srunner_create(suite);

  srunner_run_all(suite_runner, CK_NORMAL);
  failed_counter = srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  if (failed_counter == 0) {
    exit_status = EXIT_SUCCESS;
  } else {
    exit_status = EXIT_FAILURE;
  }

  return exit_status;
}
