#include <stdio.h>
#include "dokter.h" // Meng-include header dokter.h

// Implementasi fungsi dasar untuk menampilkan informasi dokter
void tampilkanInfoDokterDasar(const Dokter *dokter) {
    if (dokter != NULL) {
        printf("Nama Dokter: %s\n", dokter->nama);
        printf("Maksimal Shift per Minggu: %d\n", dokter->max_shift_per_minggu);
    } else {
        printf("Data dokter tidak valid.\n");
    }
}