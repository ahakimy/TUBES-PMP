# Nama file output
TARGET = penjadwalan

# Daftar file sumber
SRCS = main.c penjadwalan.c

# Nama file objek yang otomatis dihasilkan
OBJS = $(SRCS:.c=.o)

# Compiler dan flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

# Target default
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule untuk file .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Bersih-bersih file objek dan executable
clean:
	rm -f $(TARGET) $(OBJS)
