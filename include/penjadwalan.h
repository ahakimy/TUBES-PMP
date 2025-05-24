#ifndef PENJADWALAN_H
#define PENJADWALAN_H

#include "../include/dokter.h"

// Definisi konstanta untuk penjadwalan
#define DAYS_IN_MONTH 30
#define SHIFTS_PER_DAY 3
#define MAX_DOCTORS_PER_SHIFT 5
#define TOTAL_SHIFTS (DAYS_IN_MONTH * SHIFTS_PER_DAY)

// UBAH NILAI INI UNTUK MENGATUR JUMLAH DOKTER PER SHIFT
extern int DOCTORS_PER_SHIFT; // Dapat diubah sesuai kebutuhan

// Enums sudah didefinisikan di dokter.h, tidak perlu define ulang

// Struktur data untuk menyimpan entri jadwal (support multiple dokter per shift)
typedef struct {
    int day;
    int shift;
    int doctor_ids[MAX_DOCTORS_PER_SHIFT];
    int num_doctors;
} Schedule;

// Deklarasi variabel global untuk jadwal
extern Schedule schedule[TOTAL_SHIFTS];
extern int schedule_count;

// Fungsi utama penjadwalan
void generate_schedule(Doctor* doctors, int num_doctors);

// Fungsi pendukung penjadwalan
int is_doctor_available(Doctor* doctors, int id, int day, int shift);
int calculate_score(Doctor* doctors, int id, int day, int shift);
void assign_shift(Doctor* doctors, int day, int shift, int doctor_id);
int is_doctor_already_assigned(int day, int shift, int doctor_id);

// Fungsi utilitas
int get_week_number(int day);
int is_early_month(int day);

// Fungsi parsing untuk membaca preferensi dari CSV
ShiftType parse_shift(const char* s);
PreferensiWaktu parse_waktu(const char* s);

// Fungsi untuk menyimpan jadwal
void save_schedule_to_csv(Doctor* doctors, int num_doctors);

#endif // PENJADWALAN_H