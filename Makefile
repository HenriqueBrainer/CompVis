# CompVis - Projeto 1 (Computacao Visual)
# Makefile de build (Windows / MSYS2-MinGW).
#
# Cada integrante do grupo pode ter o SDL3/SDL3_image em uma pasta diferente.
# Em vez de editar este arquivo, crie um "Makefile.local" (nao versionado,
# ja esta no .gitignore) do lado deste Makefile com o conteudo:
#
#   SDL3_DIR       := C:/Users/SEU_USUARIO/../SDL3
#   SDL3_IMAGE_DIR := C:/Users/SEU_USUARIO/../SDL3_image

TARGET := main
EXE    := $(TARGET).exe
SRC    := main.c
CC     := gcc
CFLAGS := -Wall -Wextra -std=c99 -g

# Le overrides especificos da maquina, se existirem.
-include Makefile.local

# Defaults (usados se Makefile.local nao definir nada).
SDL3_DIR       ?= C:/msys64/ucrt64
SDL3_IMAGE_DIR ?= C:/msys64/ucrt64

CFLAGS  += -I$(SDL3_DIR)/include -I$(SDL3_IMAGE_DIR)/include
LDFLAGS += -L$(SDL3_DIR)/lib -L$(SDL3_IMAGE_DIR)/lib
LDLIBS  += -lSDL3 -lSDL3_image -lm

.PHONY: all run clean

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXE) $(LDFLAGS) $(LDLIBS)

# Uso: make run IMG=1-Testes/morango.png
IMG ?= 1-Testes/morango.png
run: $(EXE)
	./$(EXE) "$(IMG)"

clean:
	rm -f $(EXE) *.o
