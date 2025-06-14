#ifndef PENJADWALAN_H
#define PENJADWALAN_H

// Konstanta yang digunakan
#define MAX_DOCTORS 50           // Maksimum dokter
#define MAX_NAME_LEN 100         // Panjang maksimum nama dan bidang spesialisasi
#define DAYS_IN_MONTH 30         // Jumlah hari dalam sebulan
#define SHIFTS_PER_DAY 3         // Jumlah shift per hari (Pagi, Siang, Malam)
#define MAX_DOCTORS_PER_SHIFT 2  // Maksimum dokter per shift
#define MAX_WEEKS 5              // Maksimum minggu dalam sebulan (sesuai coba13.c)
#define LINE_BUFFER_SIZE 512     // Ukuran buffer untuk membaca baris CSV
#define INPUT_FILE_PENJADWALAN "data/daftar_dokter.csv"  // File input data dokter (dipertahankan)
#define OUTPUT_FILE_PENJADWALAN "data/jadwal_dokter.csv" // File output jadwal dokter (dipertahankan)

// Jenis shift, tingkat dokter, dan preferensi waktu
typedef enum { SHIFT_PAGI = 0, SHIFT_SIANG, SHIFT_MALAM } ShiftType;
typedef enum { TINGKAT_KOASS = 0, TINGKAT_RESIDEN, TINGKAT_SPESIALIS, TINGKAT_KONSULEN } TingkatDokter;
typedef enum { WAKTU_AWAL_BULAN = 0, WAKTU_AKHIR_BULAN, WAKTU_CAMPUR } PreferensiWaktu;

// Struktur data untuk menyimpan informasi dokter
typedef struct {
    char name[MAX_NAME_LEN];
    char bidang[MAX_NAME_LEN];
    TingkatDokter tingkat;
    int max_shifts_per_week;
    ShiftType preferred_shift;
    PreferensiWaktu preferred_time;
    int total_shifts_assigned;
    int weekly_shifts[MAX_WEEKS];
} Doctor;

// Struktur data untuk jadwal per shift
typedef struct {
    int day;
    int shift;
    int doctor_ids[MAX_DOCTORS_PER_SHIFT];
    int num_doctors;
} Schedule;

// Variabel global
extern Doctor doctors[MAX_DOCTORS];
extern int num_doctors;
extern Schedule schedule[DAYS_IN_MONTH * SHIFTS_PER_DAY];
extern int schedule_count;
extern int DOCTORS_PER_SHIFT;

// Deklarasi fungsi publik
int load_doctors_from_csv(const char* filename);
void generate_schedule(void);
void save_schedule(void);

#endif // PENJADWALAN_H