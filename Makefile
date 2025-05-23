# Makefile untuk Proyek Penjadwalan Dokter

# Bagian 1: Konfigurasi Variabel
# -----------------------------------------------------------------------------
# Compiler C yang akan digunakan
CC = gcc

# Direktori Sumber (.c), Header (.h), Build (.o), dan Binary (executable)
SRCDIR = src
INCDIR = include
BUILDDIR = build
BINDIR = bin

# Nama executable yang akan dihasilkan oleh proses kompilasi
TARGET = $(BINDIR)/jadwal_dokter

# Mengumpulkan semua file sumber (.c) dari direktori SRCDIR
# 'wildcard' akan mencari semua file yang cocok dengan pola
SRCS = $(wildcard $(SRCDIR)/*.c)

# Mengubah daftar file sumber (.c) menjadi daftar file objek (.o)
# 'patsubst' menggantikan akhiran .c dengan .o dan menempatkannya di BUILDDIR
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

# Flags Kompilasi untuk GCC
#   -I$(INCDIR): Memberi tahu compiler untuk mencari file header di direktori 'include'
#   -Wall: Mengaktifkan semua peringatan (sangat direkomendasikan untuk menemukan potensi masalah)
#   -g: Menghasilkan informasi debugging (penting untuk menggunakan debugger di VS Code)
CFLAGS = -I$(INCDIR) -Wall -g

# Linker Flags (bendera tambahan untuk proses linking, misal untuk library eksternal)
# Untuk proyek ini, kemungkinan tidak memerlukan library eksternal, jadi bisa dikosongkan.
LDFLAGS =

# Bagian 2: Deklarasi Target Palsu (.PHONY)
# -----------------------------------------------------------------------------
# Mendeklarasikan target-target ini sebagai 'phony'
# Ini memberitahu 'make' bahwa target-target ini bukan nama file yang sebenarnya,
# sehingga 'make' akan selalu menjalankannya meskipun ada file dengan nama yang sama.
.PHONY: all clean run

# Bagian 3: Aturan Build Utama (Target 'all')
# -----------------------------------------------------------------------------
# Target default yang akan dijalankan ketika Anda mengetik 'make' tanpa argumen.
# Tergantung pada 'dirs' (membuat direktori) dan '$(TARGET)' (executable).
all: dirs $(TARGET)

# Bagian 4: Aturan Pembuatan Direktori (Target 'dirs')
# -----------------------------------------------------------------------------
# Membuat direktori 'build' dan 'bin' jika belum ada.
# '@' menyembunyikan perintah agar tidak ditampilkan di konsol.
dirs:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(BINDIR)

# Bagian 5: Aturan Kompilasi Executable (Target '$(TARGET)')
# -----------------------------------------------------------------------------
# Aturan untuk membuat file executable akhir.
# Tergantung pada semua file objek (.o).
# $(CC)           : Menggunakan compiler (gcc)
# $(OBJS)         : Semua file objek yang telah dikompilasi
# -o $(TARGET)    : Menentukan nama dan lokasi file executable output
# $(LDFLAGS)      : Menambahkan linker flags
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Bagian 6: Aturan Kompilasi File Objek (Target '%.o')
# -----------------------------------------------------------------------------
# Aturan generik untuk mengkompilasi setiap file sumber (.c) menjadi file objek (.o).
# '%.o: %.c' berarti "untuk setiap file .o, ada file .c yang sesuai".
# '$<': Variabel otomatis yang merujuk pada prerequisit pertama (file .c).
# '$@': Variabel otomatis yang merujuk pada target (file .o).
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Bagian 7: Aturan Menjalankan Program (Target 'run')
# -----------------------------------------------------------------------------
# Target untuk mengkompilasi dan menjalankan program.
# Tergantung pada 'all' untuk memastikan program sudah terkompilasi terbaru.
run: all
	./$(TARGET)

# Bagian 8: Aturan Membersihkan Proyek (Target 'clean')
# -----------------------------------------------------------------------------
# Target untuk menghapus semua file yang dihasilkan oleh proses kompilasi.
# Berguna untuk membersihkan proyek sebelum kompilasi ulang penuh atau rilis.
clean:
	@rm -rf $(BUILDDIR) $(BINDIR)
	@echo "Cleaned!"