#ifndef DOKTER_H
#define DOKTER_H

// Definisi konstanta
#define MAX_DOCTORS 50
#define MAX_NAME_LEN 100

// Forward declaration untuk enums yang akan digunakan di penjadwalan
typedef enum {
    SHIFT_PAGI = 0,
    SHIFT_SIANG = 1, 
    SHIFT_MALAM = 2
} ShiftType;

typedef enum {
    WAKTU_AWAL_BULAN = 0,
    WAKTU_AKHIR_BULAN,
    WAKTU_CAMPUR
} PreferensiWaktu;

// Enumerasi untuk tingkat dokter
typedef enum {
    TINGKAT_KOASS = 0,
    TINGKAT_RESIDEN,
    TINGKAT_SPESIALIS,
    TINGKAT_KONSULEN
} TingkatDokter;

// Struktur data untuk menyimpan informasi dokter
typedef struct {
    char name[MAX_NAME_LEN];
    char bidang[MAX_NAME_LEN];
    TingkatDokter tingkat;
    int max_shifts_per_week;
    ShiftType preferred_shift;
    PreferensiWaktu preferred_time;
    int total_shifts_assigned;
    int weekly_shifts[5]; // Jumlah shift per minggu (untuk 5 minggu dalam sebulan)
    int shift_count[3]; // Jumlah shift pagi, siang, malam
    int violation_count; // Jumlah pelanggaran preferensi
} Doctor;

// Fungsi-fungsi untuk mengelola data dokter
int load_doctors_from_csv(const char* filename, Doctor* doctors, int max_doctors);
TingkatDokter parse_tingkat(const char* s);

#endif // DOKTER_H