#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "penjadwalan.h"

// Variabel global
Doctor doctors[MAX_DOCTORS];
int num_doctors = 0;
Schedule schedule[DAYS_IN_MONTH * SHIFTS_PER_DAY];
int schedule_count = 0;
int DOCTORS_PER_SHIFT = 2; // Bisa diubah untuk keperluan lain

// Fungsi internal: menentukan minggu ke berapa dalam sebulan
static int get_week_number(int day) {
    return day / 7;
}

// Fungsi internal: mengecek apakah termasuk awal bulan
static int is_early_month(int day) {
    return day < 15;
}

// Parsing string ke enum shift
static ShiftType parse_shift(const char* s) {
    if (strstr(s, "Pagi")) return SHIFT_PAGI;
    if (strstr(s, "Siang")) return SHIFT_SIANG;
    if (strstr(s, "Malam")) return SHIFT_MALAM;
    return SHIFT_PAGI; // Default
}

// Parsing string ke enum tingkat
static TingkatDokter parse_tingkat(const char* s) {
    if (strstr(s, "Koass")) return TINGKAT_KOASS;
    if (strstr(s, "Residen")) return TINGKAT_RESIDEN;
    if (strstr(s, "Spesialis")) return TINGKAT_SPESIALIS;
    if (strstr(s, "Konsulen")) return TINGKAT_KONSULEN;
    return TINGKAT_KOASS; // Default
}

// Parsing string ke enum preferensi waktu
static PreferensiWaktu parse_waktu(const char* s) {
    if (strstr(s, "Awal")) return WAKTU_AWAL_BULAN;
    if (strstr(s, "Akhir")) return WAKTU_AKHIR_BULAN;
    return WAKTU_CAMPUR; // Default
}

// Membaca data dokter dari file CSV
int load_doctors_from_csv(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    char line[512];
    fgets(line, sizeof(line), f); // Lewati header

    while (fgets(line, sizeof(line), f) && num_doctors < MAX_DOCTORS) {
        Doctor* d = &doctors[num_doctors];
        char* token;

        token = strtok(line, ",");
        if (!token) continue;
        strncpy(d->name, token, MAX_NAME_LEN-1);

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(d->bidang, token, MAX_NAME_LEN-1);

        token = strtok(NULL, ",");
        if (!token) continue;
        d->tingkat = parse_tingkat(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        d->max_shifts_per_week = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        d->preferred_shift = parse_shift(token);

        token = strtok(NULL, ",\n\r");
        if (!token) continue;
        d->preferred_time = parse_waktu(token);

        d->total_shifts_assigned = 0;
        memset(d->weekly_shifts, 0, sizeof(d->weekly_shifts));

        num_doctors++;
    }

    fclose(f);
    return num_doctors > 0;
}

// Mengecek apakah dokter sudah dijadwalkan di shift yang sama
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

// Menugaskan shift ke dokter tertentu
static void assign_shift(int day, int shift, int doctor_id) {
    int week = get_week_number(day);
    doctors[doctor_id].total_shifts_assigned++;
    doctors[doctor_id].weekly_shifts[week]++;
}

// Hitung skor kecocokan dokter untuk shift tertentu
static int calculate_score(int id, int day, int shift) {
    int score = 0;

    // Preferensi shift
    score += (doctors[id].preferred_shift == shift) ? 15 : -5;

    // Preferensi waktu dalam bulan
    if (doctors[id].preferred_time == WAKTU_CAMPUR) score += 5;
    else if (doctors[id].preferred_time == WAKTU_AWAL_BULAN && is_early_month(day)) score += 10;
    else if (doctors[id].preferred_time == WAKTU_AKHIR_BULAN && !is_early_month(day)) score += 10;
    else score -= 5;

    // Distribusi adil
    score += (30 - doctors[id].total_shifts_assigned);

    // Batas shift per minggu
    int week = get_week_number(day);
    int remaining = doctors[id].max_shifts_per_week - doctors[id].weekly_shifts[week];
    score += (remaining > 0) ? remaining * 10 : remaining * 20;

    // Prioritas untuk tingkat senior
    score += (int)doctors[id].tingkat * 3;

    return score;
}

// Algoritma utama untuk membuat jadwal
void generate_schedule() {
    schedule_count = 0;
    for (int day = 0; day < DAYS_IN_MONTH; day++) {
        for (int shift = 0; shift < SHIFTS_PER_DAY; shift++) {
            Schedule current_schedule = {day, shift, {-1}, 0};
            while (current_schedule.num_doctors < DOCTORS_PER_SHIFT) {
                int best_doctor = -1;
                int best_score = -9999;

                for (int i = 0; i < num_doctors; i++) {
                    if (is_doctor_already_assigned(day, shift, i, &current_schedule)) continue;
                    int score = calculate_score(i, day, shift);
                    if (score > best_score) {
                        best_score = score;
                        best_doctor = i;
                    }
                }

                if (best_doctor >= 0) {
                    current_schedule.doctor_ids[current_schedule.num_doctors++] = best_doctor;
                    assign_shift(day, shift, best_doctor);
                }
            }

            schedule[schedule_count++] = current_schedule;
        }
    }
}

// Menyimpan hasil jadwal ke file CSV
void save_schedule() {
    FILE* f = fopen("jadwal_dokter.csv", "w");
    if (!f) return;

    const char* hari[] = {"Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu", "Minggu"};
    const char* shift_names[] = {"Pagi", "Siang", "Malam"};
    const char* tingkat_str[] = {"Koass", "Residen", "Spesialis", "Konsulen"};

    fprintf(f, "Tanggal,Hari,Shift,Nama_Dokter,Bidang,Tingkat\n");
    for (int i = 0; i < schedule_count; i++) {
        int d = schedule[i].day;
        int s = schedule[i].shift;
        for (int j = 0; j < schedule[i].num_doctors; j++) {
            int id = schedule[i].doctor_ids[j];
            fprintf(f, "%d,%s,%s,%s,%s,%s\n",
                    d + 1, hari[d % 7], shift_names[s],
                    doctors[id].name, doctors[id].bidang,
                    tingkat_str[doctors[id].tingkat]);
        }
    }

    fclose(f);
}
