# Projekt SO 2026/27, temat 11 - Metro
# Budowanie: make        Uruchomienie: make run        Sprzątanie IPC: make ipc-clean

CC      ?= gcc
CFLAGS  ?= -std=c11 -D_GNU_SOURCE -Wall -Wextra -Wpedantic -Werror -O2 -g
LDFLAGS ?= -pthread

SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin

# Programy (każda rola to osobny plik wykonywalny uruchamiany przez exec)
PROGRAMS := metro zawiadowca kasa maszynista generator pasazer

# Moduły wspólne linkowane do każdego programu
COMMON := errors log params

COMMON_OBJS := $(COMMON:%=$(OBJ_DIR)/%.o)
BINS        := $(PROGRAMS:%=$(BIN_DIR)/%)

.PHONY: all clean run ipc-clean test

# Pliki .o zostają między kompilacjami (szybsze przebudowanie)
.SECONDARY: $(COMMON_OBJS) $(PROGRAMS:%=$(OBJ_DIR)/%.o)

all: $(BINS)

$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

run: all
	./$(BIN_DIR)/metro

# Usuwa struktury IPC bieżącego użytkownika, które zostały po przerwanej symulacji
ipc-clean:
	-ipcs -q | awk -v u="$$(id -un)" '$$3 == u {print $$2}' | xargs -r -n1 ipcrm -q
	-ipcs -m | awk -v u="$$(id -un)" '$$3 == u {print $$2}' | xargs -r -n1 ipcrm -m
	-ipcs -s | awk -v u="$$(id -un)" '$$3 == u {print $$2}' | xargs -r -n1 ipcrm -s

test: all
	@if [ -x tests/run_all.sh ]; then tests/run_all.sh; else echo "Testy nie są jeszcze gotowe"; fi

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

-include $(wildcard $(OBJ_DIR)/*.d)
