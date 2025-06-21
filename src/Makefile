CC 			:= gcc
CFLAGS 		:= -std=c11 -pedantic -Wall -Wextra -Werror
GUI_FLAGS 	:= -lncurses
SRC 		:= main.c

all: game

game: main.c
	$(CC) $(CFLAGS) $(SRC) $(GUI_FLAGS) -o $@

clean:
	rm -rf \
	game

.PHONY: all game
