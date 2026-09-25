# CompVis - Projeto 1 (Computacao Visual)
# Build para Windows / MSYS2-MinGW.

TARGET := main
EXE    := $(TARGET).exe
SRC    := $(wildcard src/*.c)
OBJ    := $(SRC:.c=.o)
CC     := gcc
CFLAGS = -Wall -Wextra -std=c99 -g -DSDL_MAIN_HANDLED

-include Makefile.local

SDL3_DIR       ?= C:/msys64/ucrt64
SDL3_IMAGE_DIR ?= C:/msys64/ucrt64
SDL3_TTF_DIR   ?= C:/msys64/ucrt64

CFLAGS  += -I$(SDL3_DIR)/include -I$(SDL3_IMAGE_DIR)/include -I$(SDL3_TTF_DIR)/include
LDFLAGS += -L$(SDL3_DIR)/lib -L$(SDL3_IMAGE_DIR)/lib -L$(SDL3_TTF_DIR)/lib
LDLIBS  += -lSDL3 -lSDL3_image -lSDL3_ttf -lm

.PHONY: all run clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

IMG ?= assets/imgs/morango.png
run: $(EXE)
	./$(EXE) "$(IMG)"

clean:
	rm -f $(EXE) $(OBJ)
