TARGET?=x86_64-linux-gnu
CC:=$(TARGET)-gcc-9
AS:=$(TARGET)-as
AR:=$(TARGET)-ar
LD:=$(TARGET)-ld

BINDIR:=bin
OBJDIR:=$(BINDIR)/obj
SRCDIR:=.
INCDIR:=include
BIN:=$(BINDIR)/nunsc

SRC:=$(SRCDIR)/main.c
OBJ:=$(OBJDIR)/main.o
INC:=$(INCDIR)/nunsc.h $(INCDIR)/nunsc_priv.h

CFLAGS:=-g -Wall -Wextra -pedantic -std=c11
LDFLAGS:=-lm
ASFLAGS:=

.PHONY: all build clean mrproper

all: $(BIN)

build: $(BIN)

$(BIN): $(OBJ) $(INC) $(BINDIR)
	$(CC) -o $(BIN) $(OBJ) $(LDFLAGS)

$(OBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.c $(INC) $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR)

mrproper: clean
	rm -rf $(BINDIR)
