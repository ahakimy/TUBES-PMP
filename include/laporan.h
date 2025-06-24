#ifndef UTILS_LAPORAN_H
#define UTILS_LAPORAN_H

#define MAX_ENTRIES 1000
#define MAX_NAME 100
#define MAX_LINE 1024
#define INPUT_FILE_JADWAL "data/jadwal_dokter.csv"
#define INPUT_FILE_DOKTER "data/daftar_dokter.csv"
#define INPUT_FILE_LAPORAN "data/laporan_dokter.csv"
#define OUTPUT_FILE_LAPORAN "data/laporan_dokter.csv"

typedef struct {
    int tanggal;
    char hari[20];
    char shift[10];
    char nama[MAX_NAME];
    char bidang[100];
    char tingkat[20];
} ShiftEntry;

typedef struct {
    char nama[MAX_NAME];
    char bidang[100];
    char tingkat[20];
    int max_shift_per_minggu;
    char preferensi_shift[10];
    char preferensi_waktu[20];
} DoctorPref;

extern ShiftEntry shift_data[MAX_ENTRIES];
extern DoctorPref doctor_data[MAX_ENTRIES];
extern int total_shift_entries;
extern int total_doctor_entries;
extern int total_shift[MAX_ENTRIES];
extern int shift_mingguan[MAX_ENTRIES][5];
extern int shift_count_per_type[MAX_ENTRIES][3];
extern int pelanggaran[MAX_ENTRIES];

// Fungsi
void load_shift_data(const char *filename);
void load_doctor_data(const char *filename);
int find_doctor_index(const char *nama);
int get_shift_index(const char *shift);
int is_waktu_sesuai(const char *pref, int tanggal);
void proses_data();
void tulis_laporan(const char *filename);
void lihat_jumlah_shift();
void lihat_pelanggaran();
void lihat_jumlah_pelanggaran_per_dokter();
void lihat_pelanggaran_dokter();

#endif