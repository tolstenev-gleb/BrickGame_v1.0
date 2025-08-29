#include "tetris.h"

#include <check.h>

#include "../brick_game.h"

// FSM    -> kStart
// action -> Start
START_TEST(onStartStateGetStart) {
  // Arrange
  initState();
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
  initState();
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
START_TEST(onStartStateGetTerminate) {
  // Arrange
  initState();
  setState(kMoving);
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
// action -> Right

// FSM    -> kMoving
// action -> Action

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
  initState();
  setState(kGameOver);
  TetrisInfo_t *game = getTetrisInfo();
  GameInfo_t game_info = {0};
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
  initState();
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

  tcase_add_test(tc_core, onStartStateGetStart);
  tcase_add_test(tc_core, onStartStateGetTerminate);

  tcase_add_test(tc_core, onGameOverStateGetStart);
  tcase_add_test(tc_core, onGameOverStateGetTerminate);

  suite_add_tcase(suite, tc_core);

  return suite;
}

int main(void) {
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
