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

UICOMMON =  states/hiscores.o \
			states/playdemo.o \
			states/game.o \
			ui.o \
			os/linux_funs.o
UICOMMON := $(addprefix $(ODIR)/ui/, $(UICOMMON))

CURSES = init.o \
		 functions.o
CURSES := $(addprefix $(ODIR)/ui/curses/, $(CURSES))


BUILD = build
OUT = $(BUILD)/tetr

all: dir $(OUT)

dir:
	-mkdir -p build
	-mkdir -p $(ODIR)
	-mkdir -p $(ODIR)/core
	-mkdir -p $(ODIR)/ui/curses $(ODIR)/ui/os $(ODIR)/ui/states

$(OUT): $(SRC)/main.c $(CORE) $(UICOMMON) $(CURSES)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(ODIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm -rf $(ODIR)
	-rm -rf $(BUILD)
