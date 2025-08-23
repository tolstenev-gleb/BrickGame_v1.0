CC 			:= gcc
CFLAGS 		:= -std=c11 -pedantic -pthread
GUI_FLAGS 	:= -lncurses
SRC_MAIN	:= main_cli.c
OBJ_MAIN	:= main_cli.o
SRC_GUI_CLI	:= gui/cli/cli.c
OBJ_CLI		:= gui/cli/cli.o
HDR_GUI_CLI	:= gui/cli/cli.h
LIB_TETRIS	:= brick_game/tetris/tetris.a
OBJ_TETRIS	:= brick_game/tetris/tetris.o
SRC_TETRIS	:= brick_game/tetris/tetris.c
HDR_TETRIS	:= brick_game/tetris/tetris.h

all: game

game: $(OBJ_MAIN) $(OBJ_CLI) $(LIB_TETRIS)
	$(CC) $(CFLAGS) $(MACROS) $(OBJ_MAIN) $(OBJ_CLI) $(LIB_TETRIS) $(GUI_FLAGS) -o $@

$(OBJ_MAIN): $(SRC_MAIN)
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

$(OBJ_CLI): $(SRC_GUI_CLI) $(HDR_GUI_CLI)
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

$(OBJ_TETRIS): $(SRC_TETRIS) $(HDR_TETRIS)
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

$(LIB_TETRIS): $(OBJ_TETRIS)
	ar rcs $@ $<

# help: $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(HDR_GUI_CLI) $(HDR_TETRIS)
# 	$(CC) $(CFLAGS) -DHELP $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(GUI_FLAGS) -o $@

# nolimits: $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(HDR_GUI_CLI) $(HDR_TETRIS)
# 	$(CC) $(CFLAGS) -DNO_LIMITS $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(GUI_FLAGS) -o $@

# debug: $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(HDR_GUI_CLI) $(HDR_TETRIS)
# 	$(CC) $(CFLAGS) -DDEBUG $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(GUI_FLAGS) -o $@

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

.PHONY: all game
