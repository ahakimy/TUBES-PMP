#ifndef PENJADWALAN_H
#define PENJADWALAN_H

// Konstanta umum
#define MAX_DOCTORS 50
#define MAX_NAME_LEN 100
#define DAYS_IN_MONTH 30
#define SHIFTS_PER_DAY 3
#define MAX_DOCTORS_PER_SHIFT 2

// Jenis shift dalam sehari
typedef enum { SHIFT_PAGI = 0, SHIFT_SIANG, SHIFT_MALAM } ShiftType;

// Jenjang/tingkat dokter
typedef enum {
    TINGKAT_KOASS = 0,
    TINGKAT_RESIDEN,
    TINGKAT_SPESIALIS,
    TINGKAT_KONSULEN
} TingkatDokter;

// Preferensi waktu kerja dalam sebulan
typedef enum {
    WAKTU_AWAL_BULAN = 0,
    WAKTU_AKHIR_BULAN,
    WAKTU_CAMPUR
} PreferensiWaktu;

// Struktur data untuk menyimpan informasi dokter
typedef struct {
    char name[MAX_NAME_LEN];
    char bidang[MAX_NAME_LEN];
    TingkatDokter tingkat;
    int max_shifts_per_week;
    ShiftType preferred_shift;
    PreferensiWaktu preferred_time;
    int total_shifts_assigned;
    int weekly_shifts[5]; // Shift yang dijadwalkan per minggu (asumsi: 5 minggu/bulan)
} Doctor;

// Struktur data untuk menyimpan satu shift dalam jadwal
typedef struct {
    int day;
    int shift;
    int doctor_ids[MAX_DOCTORS_PER_SHIFT];
    int num_doctors;
} Schedule;

// Variabel global (akan di-definisikan di penjadwalan.c)
extern Doctor doctors[MAX_DOCTORS];
extern int num_doctors;
extern Schedule schedule[DAYS_IN_MONTH * SHIFTS_PER_DAY];
extern int schedule_count;

// Deklarasi fungsi-fungsi utama
int load_doctors_from_csv(const char* filename);
void generate_schedule();
void save_schedule();

#endif
