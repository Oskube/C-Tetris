CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lncurses

SRC = src
ODIR = obj

CORE = game_randomisers.o \
	   game.o \
	   file_misc.o \
	   hiscore.o \
	   demo.o
CORE := $(addprefix $(ODIR)/core/, $(CORE))

CURSES = state_hiscores.o \
		 game_functions.o \
		 state_game.o \
		 curses_main.o \
		 os_linux.o
CURSES := $(addprefix $(ODIR)/ui_curses/, $(CURSES))


BUILD = build
OUT = $(BUILD)/tetr

all: dir $(OUT)

dir:
	-mkdir -p build
	-mkdir -p $(ODIR)
	-mkdir -p $(ODIR)/core
	-mkdir -p $(ODIR)/ui_curses

$(OUT): $(SRC)/main.c $(CORE) $(CURSES)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(ODIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm -rf $(ODIR)
	-rm -rf $(BUILD)
