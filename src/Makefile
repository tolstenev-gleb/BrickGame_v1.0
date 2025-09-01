CC 			:= gcc
CFLAGS 		:= -std=c11 -pedantic -pthread
GUI_FLAGS 	:= -lncurses
MACROS		:= # -DHELP # -DDEBUG # -DNO_LIMITS

SRC_MAIN	:= main_cli.c
OBJ_MAIN	:= main_cli.o
SRC_GUI_CLI	:= gui/cli/cli.c
OBJ_CLI		:= gui/cli/cli.o
HDR_GUI_CLI	:= gui/cli/cli.h
LIB_TETRIS	:= brick_game/tetris/tetris.a
OBJ_TETRIS	:= brick_game/tetris/tetris.o
SRC_TETRIS	:= brick_game/tetris/tetris.c
HDR_TETRIS	:= brick_game/tetris/tetris.h
HDR_API		:= brick_game/brick_game.h

TEST		:= tetris_test
SRC_TEST	:= brick_game/tetris/tetris_test.c
GCOVFLAGS	:= -fprofile-arcs -ftest-coverage
TEST_GCOV	:= gcov_test_tetris
REPORT_DIR	:= gcov_report
OS			:= $(shell uname)
CHECK_FLAGS	:= -lcheck
ifeq ($(OS), Linux)
    CHECK_FLAGS += -lpthread -lrt -lm -lsubunit
endif

FILE_SAVE	:= highscore_tetris.txt

all: game

game: $(OBJ_MAIN) $(OBJ_CLI) $(LIB_TETRIS) $(FILE_SAVE)
	$(CC) $(CFLAGS) $(MACROS) $(OBJ_MAIN) $(OBJ_CLI) $(LIB_TETRIS) $(GUI_FLAGS) -o $@

$(OBJ_MAIN): $(SRC_MAIN)
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

$(OBJ_CLI): $(SRC_GUI_CLI) $(HDR_GUI_CLI) $(HDR_API)
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

$(OBJ_TETRIS): $(SRC_TETRIS) $(HDR_TETRIS) $(HDR_API)
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

$(FILE_SAVE):
	touch $(FILE_SAVE)

lib: $(LIB_TETRIS)

$(LIB_TETRIS): $(OBJ_TETRIS)
	ar rcs $@ $<

test: $(SRC_TETRIS) $(SRC_TEST)
	$(CC) $^ $(CHECK_FLAGS) -o $(TEST) 
	./$(TEST)

test_print: $(SRC_TETRIS) $(SRC_TEST)
	$(CC) -DPRINT_TEST $^ $(CHECK_FLAGS) -o $(TEST) 
	./$(TEST)

gcov_report: $(SRC_TEST) $(SRC_TETRIS) 
	$(CC) $(GCOVFLAGS) $^ $(CHECK_FLAGS) -o $(TEST_GCOV)
	./$(TEST_GCOV)
	lcov -t "$(TEST_GCOV)" --exclude $(SRC_TEST) -o $(TEST_GCOV).info -c -d .
	genhtml -o $@ $(TEST_GCOV).info
	rm -rf *.gcno *.gcda $(TEST_GCOV) $(TEST_GCOV).info
# 	open ./$(REPORT_DIR)/index.html\

clean:
	rm -rf \
	game \
	help \
	nolimits \
	debug \
	$(LIB_TETRIS) \
	$(OBJ_TETRIS) \
	$(OBJ_MAIN) \
	$(OBJ_CLI) \
	$(TEST) \
	$(TEST_GCOV) \
	*.gcno *.gcda $(REPORT_DIR) 

re:
	$(MAKE) clean
	$(MAKE) game

.PHONY: all clean gcov_report
