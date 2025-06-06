#ifndef PENJADWALAN_H
#define PENJADWALAN_H

#define MAX_DOCTORS 50
#define MAX_NAME_LEN 100
#define DAYS_IN_MONTH 30
#define SHIFTS_PER_DAY 3
#define MAX_DOCTORS_PER_SHIFT 2

// Enum untuk jenis shift, tingkat dokter, dan preferensi waktu
typedef enum { SHIFT_PAGI = 0, SHIFT_SIANG, SHIFT_MALAM } ShiftType;
typedef enum { TINGKAT_KOASS = 0, TINGKAT_RESIDEN, TINGKAT_SPESIALIS, TINGKAT_KONSULEN } TingkatDokter;
typedef enum { WAKTU_AWAL_BULAN = 0, WAKTU_AKHIR_BULAN, WAKTU_CAMPUR } PreferensiWaktu;

// Struktur data dokter
typedef struct {
    char name[MAX_NAME_LEN];
    char bidang[MAX_NAME_LEN];
    TingkatDokter tingkat;
    int max_shifts_per_week;
    ShiftType preferred_shift;
    PreferensiWaktu preferred_time;
    int total_shifts_assigned;
    int weekly_shifts[5];
} Doctor;

// Struktur data jadwal per shift
typedef struct {
    int day;
    int shift;
    int doctor_ids[MAX_DOCTORS_PER_SHIFT];
    int num_doctors;
} Schedule;

// Variabel global (diimplementasikan di penjadwalan.c)
extern Doctor doctors[MAX_DOCTORS];
extern int num_doctors;
extern Schedule schedule[DAYS_IN_MONTH * SHIFTS_PER_DAY];
extern int schedule_count;
extern int DOCTORS_PER_SHIFT;

// Fungsi-fungsi publik
int load_doctors_from_csv(const char* filename);
void generate_schedule(void);
void save_schedule(void);

#endif // PENJADWALAN_H
