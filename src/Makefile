CC 			:= gcc
CFLAGS 		:= -std=c11 -pedantic -pthread
GUI_FLAGS 	:= -lncurses
MACROS		:= -DHELP # -DDEBUG # -DNO_LIMITS
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

all: game

game: $(OBJ_MAIN) $(OBJ_CLI) $(LIB_TETRIS)
	$(CC) $(CFLAGS) $(MACROS) $(OBJ_MAIN) $(OBJ_CLI) $(LIB_TETRIS) $(GUI_FLAGS) -o $@

$(OBJ_MAIN): $(SRC_MAIN)
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

$(OBJ_CLI): $(SRC_GUI_CLI) $(HDR_GUI_CLI) $(HDR_API)
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

$(OBJ_TETRIS): $(SRC_TETRIS) $(HDR_TETRIS) $(HDR_API)
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

lib: $(LIB_TETRIS)

$(LIB_TETRIS): $(OBJ_TETRIS)
	ar rcs $@ $<

clean:
	rm -rf \
	game \
	help \
	nolimits \
	debug \
	$(LIB_TETRIS) \
	$(OBJ_TETRIS) \
	$(OBJ_MAIN) \
	$(OBJ_CLI)

re:
	$(MAKE) clean
	$(MAKE) game

.PHONY: all clean
