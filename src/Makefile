CC 			:= gcc
CFLAGS 		:= -std=c11 -pedantic -pthread
GUI_FLAGS 	:= -lncurses
SRC_MAIN	:= main.c
SRC_GUI_CLI	:= gui/cli/cli.c
HDR_GUI_CLI	:= gui/cli/cli.h
SRC_TETRIS	:= brick_game/tetris/tetris.c
HDR_TETRIS	:= brick_game/tetris/tetris.h

all: game

game: $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(HDR_GUI_CLI) $(HDR_TETRIS)
	$(CC) $(CFLAGS) $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(GUI_FLAGS) -o $@

help: $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(HDR_GUI_CLI) $(HDR_TETRIS)
	$(CC) $(CFLAGS) -DHELP $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(GUI_FLAGS) -o $@

nolimits: $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(HDR_GUI_CLI) $(HDR_TETRIS)
	$(CC) $(CFLAGS) -DNO_LIMITS $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(GUI_FLAGS) -o $@

debug: $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(HDR_GUI_CLI) $(HDR_TETRIS)
	$(CC) $(CFLAGS) -DDEBUG $(SRC_MAIN) $(SRC_GUI_CLI) $(SRC_TETRIS) $(GUI_FLAGS) -o $@

clean:
	rm -rf \
	game \
	help \
	nolimits \
	debug 

.PHONY: all game
