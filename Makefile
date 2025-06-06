# Makefile untuk mengompilasi program penjadwalan dokter

# Compiler yang digunakan
CC = gcc

# Flag untuk kompilasi (mengaktifkan peringatan dan standar C99)
CFLAGS = -Wall -std=c99 -Iinclude

# Direktori untuk file sumber, header, dan output
SRC_DIR = src
INCLUDE_DIR = include
BIN_DIR = bin

# Nama file output (executable)
TARGET = $(BIN_DIR)/scheduler

# Daftar file sumber di folder src/
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Daftar file objek yang dihasilkan dari file sumber
OBJECTS = $(SOURCES:.c=.o)

# Header file yang menjadi dependensi
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

# Aturan default: membangun executable
all: $(TARGET)

# Aturan untuk membuat executable dari file objek
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJECTS) -o $(TARGET)

# Aturan untuk mengompilasi file sumber menjadi file objek
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Aturan untuk membersihkan file hasil kompilasi
clean:
	rm -f $(OBJECTS) $(TARGET)

# Aturan untuk menjalankan program setelah dikompilasi
run: $(TARGET)
	./$(TARGET)

# Aturan untuk menandai bahwa 'clean' dan 'run' bukan file
.PHONY: clean run