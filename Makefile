CC = gcc
CFLAGS = -Wall -Wextra -Iinclude `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

SRC = src/main.c src/gui.c \
      src/dokter.c src/laporan.c src/laporan1.c src/penjadwalan.c
OBJ = $(SRC:.c=.o)
BIN = bin/penjadwalan

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f build/*.o $(BIN)