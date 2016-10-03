CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lncurses
SDL = `sdl2-config --cflags --libs`

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

UISDL = init.o \
		functions.o
UISDL := $(addprefix $(ODIR)/ui/sdl/, $(UISDL))
LIBS += $(SDL)


BUILD = build
OUT = $(BUILD)/tetr

all: dir $(OUT)

dir:
	-mkdir -p build
	-mkdir -p $(ODIR)
	-mkdir -p $(ODIR)/core
	-mkdir -p $(ODIR)/ui/curses $(ODIR)/ui/os $(ODIR)/ui/states $(ODIR)/ui/sdl

$(OUT): $(SRC)/main.c $(CORE) $(UICOMMON) $(CURSES) $(UISDL)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(ODIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@  `sdl2-config --cflags`

clean:
	-rm -rf $(ODIR)
	-rm -rf $(BUILD)
