# Output binary
TARGET = bin/penjadwalan

# Direktori
SRC_DIR = src
INC_DIR = include
OBJ_DIR = build

# Semua file .c di src
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Ganti ekstensi jadi .o dan simpan di build/
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Compiler dan flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I$(INC_DIR)

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

# Compile tiap .c jadi .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object dan binary
clean:
	rm -rf $(OBJ_DIR) bin

# Run (jalankan program setelah make)
run: all
	./$(TARGET)
