#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/penjadwalan.h"

// Variabel global
Doctor doctors[MAX_DOCTORS];
int num_doctors = 0;
Schedule schedule[DAYS_IN_MONTH * SHIFTS_PER_DAY];
int schedule_count = 0;
int DOCTORS_PER_SHIFT = 2;

// Deklarasi fungsi pembantu
static int get_week_number(int day);
static int is_early_month(int day);
static int is_doctor_available(int id, int day, int shift);
static int is_doctor_already_assigned(int day, int shift, int doctor_id, Schedule* current_schedule);
static int calculate_score(int id, int day, int shift);
static void assign_shift(int day, int shift, int doctor_id);
static ShiftType parse_shift(const char* s);
static TingkatDokter parse_tingkat(const char* s);
static PreferensiWaktu parse_waktu(const char* s);

// Fungsi untuk memuat data dokter dari file CSV
int load_doctors_from_csv(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    char line[LINE_BUFFER_SIZE];
    if (!fgets(line, LINE_BUFFER_SIZE, f)) {
        fclose(f);
        return 0;
    }

    while (fgets(line, LINE_BUFFER_SIZE, f) && num_doctors < MAX_DOCTORS) {
        Doctor* d = &doctors[num_doctors];
        char* token;

        // Hapus karakter newline atau carriage return
        line[strcspn(line, "\r\n")] = '\0';
       
        // Parsing nama dokter
        token = strtok(line, ",");
        if (!token || strlen(token) == 0 || strlen(token) >= MAX_NAME_LEN) continue;
        strncpy(d->name, token, MAX_NAME_LEN - 1);
        d->name[MAX_NAME_LEN - 1] = '\0';

        // Parsing bidang spesialisasi
        token = strtok(NULL, ",");
        if (!token || strlen(token) == 0 || strlen(token) >= MAX_NAME_LEN) continue;
        strncpy(d->bidang, token, MAX_NAME_LEN - 1);
        d->bidang[MAX_NAME_LEN - 1] = '\0';

        // Parsing tingkat dokter
        token = strtok(NULL, ",");
        if (!token) continue;
        d->tingkat = parse_tingkat(token);

        // Parsing maksimum shift per minggu
        token = strtok(NULL, ",");
        if (!token) continue;
        int max_shifts = atoi(token);
        if (max_shifts <= 0) continue;
        d->max_shifts_per_week = max_shifts;

        // Parsing preferensi shift
        token = strtok(NULL, ",");
        if (!token) continue;
        d->preferred_shift = parse_shift(token);

        // Parsing preferensi waktu
        token = strtok(NULL, ",\n\r");
        if (!token) continue;
        d->preferred_time = parse_waktu(token);

        // Inisialisasi statistik dokter
        d->total_shifts_assigned = 0;
        memset(d->weekly_shifts, 0, sizeof(d->weekly_shifts));
        num_doctors++;
    }

    fclose(f);
    if (num_doctors == 0) return 0;
    return 1;
}

// Fungsi untuk menghasilkan jadwal bulanan menggunakan algoritma greedy
void generate_schedule(void) {
    schedule_count = 0;

    for (int day = 0; day < DAYS_IN_MONTH; day++) {
        for (int shift = 0; shift < SHIFTS_PER_DAY; shift++) {
            Schedule current_schedule = { .day = day, .shift = shift, .num_doctors = 0 };
            for (int i = 0; i < MAX_DOCTORS_PER_SHIFT; i++) {
                current_schedule.doctor_ids[i] = -1;
            }

            while (current_schedule.num_doctors < DOCTORS_PER_SHIFT) {
                int best_doctor = -1;
                int best_score = -9999;

                // Pilih dokter terbaik dengan mematuhi batas shift
                for (int i = 0; i < num_doctors; i++) {
                    if (!is_doctor_available(i, day, shift)) continue;
                    if (is_doctor_already_assigned(day, shift, i, &current_schedule)) continue;

                    int score = calculate_score(i, day, shift);
                    if (score > best_score) {
                        best_score = score;
                        best_doctor = i;
                    }
                }

                // Jika tidak ada dokter tersedia, izinkan kelebihan shift
                if (best_doctor < 0) {
                    for (int i = 0; i < num_doctors; i++) {
                        if (is_doctor_already_assigned(day, shift, i, &current_schedule)) continue;

                        int score = calculate_score(i, day, shift);
                        if (score > best_score) {
                            best_score = score;
                            best_doctor = i;
                        }
                    }
                }

                // Tetapkan dokter ke shift jika ditemukan
                if (best_doctor >= 0) {
                    current_schedule.doctor_ids[current_schedule.num_doctors] = best_doctor;
                    current_schedule.num_doctors++;
                    assign_shift(day, shift, best_doctor);
                } else {
                    break; // Tidak ada dokter yang bisa ditugaskan
                }
            }

            schedule[schedule_count++] = current_schedule;
        }
    }
}

// Fungsi untuk menyimpan jadwal ke file CSV
void save_schedule(void) {
    FILE* f = fopen(OUTPUT_FILE, "w");
    if (!f) return;

    fprintf(f, "Tanggal,Hari,Shift,Nama_Dokter,Bidang,Tingkat\n");

    const char* hari[] = {"Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu", "Minggu"};
    const char* shift_names[] = {"Pagi", "Siang", "Malam"};
    const char* tingkat_str[] = {"Koass", "Residen", "Spesialis", "Konsulen"};

    for (int i = 0; i < schedule_count; i++) {
        int d = schedule[i].day;
        int s = schedule[i].shift;

        for (int j = 0; j < schedule[i].num_doctors; j++) {
            int id = schedule[i].doctor_ids[j];
            if (id >= 0) {
                fprintf(f, "%d,%s,%s,%s,%s,%s\n",
                        d + 1, hari[d % 7], shift_names[s],
                        doctors[id].name, doctors[id].bidang,
                        tingkat_str[doctors[id].tingkat]);
            }
        }
    }

    fclose(f);
}

// Fungsi pembantu: Menghitung minggu ke berapa dari hari tertentu
static int get_week_number(int day) {
    return day / 7;
}

// Fungsi pembantu: Mengecek apakah hari berada di awal bulan (< 15)
static int is_early_month(int day) {
    return day < 15;
}

// Fungsi pembantu: Mengecek ketersediaan dokter (selalu tersedia, overwork ditangani di scoring)
static int is_doctor_available(int id, int day, int shift) {
    if (id >= num_doctors) return 0;
    return 1;
}

// Fungsi pembantu: Mengecek apakah dokter sudah dijadwalkan pada hari dan shift tertentu
static int is_doctor_already_assigned(int day, int shift, int doctor_id, Schedule* current_schedule) {
    for (int i = 0; i < schedule_count; i++) {
        if (schedule[i].day == day && schedule[i].shift == shift) {
            for (int j = 0; j < schedule[i].num_doctors; j++) {
                if (schedule[i].doctor_ids[j] == doctor_id) return 1;
            }
        }
    }

    for (int j = 0; j < current_schedule->num_doctors; j++) {
        if (current_schedule->doctor_ids[j] == doctor_id) return 1;
    }

    return 0;
}

// Fungsi pembantu: Menghitung skor dokter berdasarkan preferensi dan statistik
static int calculate_score(int id, int day, int shift) {
    int score = 0;

    if (doctors[id].preferred_shift == shift) score += 15;
    else score -= 5;

    if (doctors[id].preferred_time == WAKTU_CAMPUR) score += 5;
    else if (doctors[id].preferred_time == WAKTU_AWAL_BULAN && is_early_month(day)) score += 10;
    else if (doctors[id].preferred_time == WAKTU_AKHIR_BULAN && !is_early_month(day)) score += 10;
    else score -= 5;

    score += (30 - doctors[id].total_shifts_assigned);

    int week = get_week_number(day);
    int remaining_shifts = doctors[id].max_shifts_per_week - doctors[id].weekly_shifts[week];
    if (remaining_shifts > 0) {
        score += remaining_shifts * 10;
    } else {
        score += remaining_shifts * 20;
    }

    score += (int)doctors[id].tingkat * 3;

    return score;
}

// Fungsi pembantu: Menetapkan shift ke dokter dan memperbarui statistik
static void assign_shift(int day, int shift, int doctor_id) {
    int week = get_week_number(day);
    doctors[doctor_id].total_shifts_assigned++;
    doctors[doctor_id].weekly_shifts[week]++;
}

// Fungsi pembantu: Mengonversi string shift ke enum
static ShiftType parse_shift(const char* s) {
    if (!s) return SHIFT_PAGI;
    if (strstr(s, "Pagi")) return SHIFT_PAGI;
    if (strstr(s, "Siang")) return SHIFT_SIANG;
    if (strstr(s, "Malam")) return SHIFT_MALAM;
    return SHIFT_PAGI;
}

// Fungsi pembantu: Mengonversi string tingkat dokter ke enum
static TingkatDokter parse_tingkat(const char* s) {
    if (!s) return TINGKAT_KOASS;
    if (strstr(s, "Koass")) return TINGKAT_KOASS;
    if (strstr(s, "Residen")) return TINGKAT_RESIDEN;
    if (strstr(s, "Spesialis")) return TINGKAT_SPESIALIS;
    if (strstr(s, "Konsulen")) return TINGKAT_KONSULEN;
    return TINGKAT_KOASS;
}

// Fungsi pembantu: Mengonversi string preferensi waktu ke enum
static PreferensiWaktu parse_waktu(const char* s) {
    if (!s) return WAKTU_CAMPUR;
    if (strstr(s, "Awal")) return WAKTU_AWAL_BULAN;
    if (strstr(s, "Akhir")) return WAKTU_AKHIR_BULAN;
    if (strstr(s, "Campur")) return WAKTU_CAMPUR;
    return WAKTU_CAMPUR;
}