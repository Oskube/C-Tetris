CC = gcc
CFLAGS = -Wall -g
LIBS = -lncurses

SRC = src
ODIR = obj
OBJS = $(ODIR)/game_randomisers.o \
	   $(ODIR)/game.o \
	   $(ODIR)/UI_curses.o

BUILD = build
OUT = $(BUILD)/tetr

all: dir $(OUT)

dir:
	-mkdir -p build
	-mkdir -p $(ODIR)

$(OUT): $(SRC)/main.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(ODIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm -rf $(ODIR)
	-rm -rf $(BUILD)
