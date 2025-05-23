#ifndef DOKTER_H
#define DOKTER_H

// Definisi konstanta untuk ukuran buffer
#define MAX_NAMA_DOKTER 50

// Definisi struktur untuk menyimpan data dokter
typedef struct {
    char nama[MAX_NAMA_DOKTER];
    int max_shift_per_minggu;
    // Preferensi shift akan kita tambahkan nanti
} Dokter;

// Deklarasi fungsi dasar untuk Modul Dokter
// Parameter:
//   - dokter: Pointer ke struktur Dokter yang akan ditampilkan
void tampilkanInfoDokterDasar(const Dokter *dokter);

#endif // DOKTER_H