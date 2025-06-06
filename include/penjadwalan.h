#ifndef PENJADWALAN_H
#define PENJADWALAN_H

// Konstanta untuk batasan data
#define MAX_DOCTORS 50 // Maksimum jumlah dokter
#define MAX_NAME_LEN 100 // Panjang maksimum nama dan bidang
#define DAYS_IN_MONTH 30 // Jumlah hari dalam sebulan
#define SHIFTS_PER_DAY 3 // Jumlah shift per hari (pagi, siang, malam)
#define MAX_DOCTORS_PER_SHIFT 2 // Maksimum dokter per shift
#define INPUT_FILE "data/daftar_dokter.csv" // File input data dokter
#define OUTPUT_FILE "data/jadwal_dokter.csv" // File output jadwal

// Enum untuk jenis shift, tingkat dokter, dan preferensi waktu
typedef enum { SHIFT_PAGI = 0, SHIFT_SIANG, SHIFT_MALAM } ShiftType; // Jenis shift
typedef enum { TINGKAT_KOASS = 0, TINGKAT_RESIDEN, TINGKAT_SPESIALIS, TINGKAT_KONSULEN } TingkatDokter; // Tingkat dokter
typedef enum { WAKTU_AWAL_BULAN = 0, WAKTU_AKHIR_BULAN, WAKTU_CAMPUR } PreferensiWaktu; // Preferensi waktu

// Struktur data dokter
typedef struct {
    char name[MAX_NAME_LEN]; // Nama dokter
    char bidang[MAX_NAME_LEN]; // Bidang spesialisasi
    TingkatDokter tingkat; // Tingkat dokter
    int max_shifts_per_week; // Maksimum shift per minggu
    ShiftType preferred_shift; // Shift yang disukai
    PreferensiWaktu preferred_time; // Waktu yang disukai
    int total_shifts_assigned; // Total shift yang sudah ditugaskan
    int weekly_shifts[5]; // Shift per minggu (5 minggu)
} Doctor;

// Struktur data jadwal per shift
typedef struct {
    int day; // Hari ke berapa
    int shift; // Jenis shift
    int doctor_ids[MAX_DOCTORS_PER_SHIFT]; // ID dokter yang ditugaskan
    int num_doctors; // Jumlah dokter dalam shift
} Schedule;

// Variabel global (diimplementasikan di penjadwalan.c)
extern Doctor doctors[MAX_DOCTORS]; // Array data dokter
extern int num_doctors; // Jumlah dokter
extern Schedule schedule[DAYS_IN_MONTH * SHIFTS_PER_DAY]; // Array jadwal
extern int schedule_count; // Jumlah entri jadwal
extern int DOCTORS_PER_SHIFT; // Jumlah dokter per shift

// Fungsi-fungsi publik
int load_doctors_from_csv(const char* filename); // Muat data dokter dari CSV
void generate_schedule(void); // Buat jadwal otomatis
void save_schedule(void); // Simpan jadwal ke CSV

#endif // PENJADWALAN_H