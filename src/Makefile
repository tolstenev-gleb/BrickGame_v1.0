CC 			:= gcc
CFLAGS 		:= -std=c11 -pedantic -pthread
GUI_FLAGS 	:= -lncurses
SRC 		:= main.c

all: game

game: main.c
	$(CC) $(CFLAGS) $(SRC) $(GUI_FLAGS) -o $@

debug: main.c
	$(CC) $(CFLAGS) -DDEBUG $(SRC) $(GUI_FLAGS) -o $@

clean:
	rm -rf \
	game \
	debug 

.PHONY: all game
