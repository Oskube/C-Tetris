CC = gcc
CFLAGS = -Wall -Wextra

SRC = src
ODIR = obj

CORE = game_randomisers.o \
	   game.o \
	   file_misc.o \
	   hiscore.o \
	   demo.o
CORE := $(addprefix $(ODIR)/core/, $(CORE))

UI =  states/hiscores.o \
	  states/playdemo.o \
	  states/game.o \
	  states/common.o \
	  ui.o \
	  os/linux_funs.o
UI := $(addprefix $(ODIR)/ui/, $(UI))

CURSES = init.o \
		 functions.o
CURSES := $(addprefix $(ODIR)/ui/curses/, $(CURSES))

UISDL = init.o \
		functions.o
UISDL := $(addprefix $(ODIR)/ui/sdl/, $(UISDL))


BUILD = build
OUT = $(BUILD)/tetr

.PHONY: all release debug clean dir only-curses

release: all

debug: CFLAGS += -g
debug: all

.SECONDEXPANSION:
all: UI += $(CURSES) $(UISDL)
all: LIBS += -lncurses `sdl2-config --cflags --libs`
all: dir $$(UI) $(OUT)

only-curses: UI += $(CURSES)
only-curses: LIBS += -lncurses
only-curses: CFLAGS += -D _NO_SDL
only-curses: dir $$(UI) $(OUT)

dir:
	-mkdir -p build
	-mkdir -p $(ODIR)
	-mkdir -p $(ODIR)/core
	-mkdir -p $(ODIR)/ui/os $(ODIR)/ui/states
	-mkdir -p $(ODIR)/ui/curses $(ODIR)/ui/sdl
	cp ./res/* ./build/

$(OUT): $(SRC)/main.c $(CORE)
	$(CC) $(CFLAGS) $^ $(UI) -o $@ $(LIBS)

$(ODIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LIBS)

clean:
	-rm -rf $(ODIR)
	-rm -rf $(BUILD)
