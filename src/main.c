#include <stdio.h>
#include <string.h> // Diperlukan untuk fungsi strcpy
#include "dokter.h" // Meng-include header dokter.h

int main() {
    printf("Selamat datang di Aplikasi Penjadwalan Dokter!\n");
    printf("--- Uji Coba Modul Dokter Dasar ---\n\n");

    // Membuat objek Dokter
    Dokter dokterUji;

    // Mengisi data dokter uji
    // Menggunakan strcpy karena nama adalah array of char (string)
    strcpy(dokterUji.nama, "Dr. Siti Aminah");
    dokterUji.max_shift_per_minggu = 6;

    // Memanggil fungsi dari modul dokter.c melalui header dokter.h
    tampilkanInfoDokterDasar(&dokterUji);

    printf("\n--- Uji Coba Selesai ---\n");
    return 0;
}