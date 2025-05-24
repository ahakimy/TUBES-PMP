#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/penjadwalan.h"
#include "../include/dokter.h"

// Definisi variabel global
int DOCTORS_PER_SHIFT = 2; // Default 2 dokter per shift, dapat diubah
Schedule schedule[TOTAL_SHIFTS];
int schedule_count = 0;

// Fungsi untuk menghasilkan jadwal otomatis
void generate_schedule(Doctor* doctors, int num_doctors) {
    schedule_count = 0;
    
    // Initialize random seed
    srand((unsigned)time(NULL));

    // Loop untuk setiap hari dalam sebulan
    for (int day = 0; day < DAYS_IN_MONTH; ++day) {
        // Loop untuk setiap shift dalam sehari
        for (int shift = 0; shift < SHIFTS_PER_DAY; ++shift) {
            Schedule current_schedule = {day, shift, {-1}, 0};
            
            // Cari multiple dokter terbaik untuk shift ini
            for (int slot = 0; slot < DOCTORS_PER_SHIFT; slot++) {
                int best_doctor = -1;
                int best_score = -9999; // Skor awal yang sangat rendah

                // Loop untuk setiap dokter untuk menemukan dokter terbaik untuk slot ini
                for (int i = 0; i < num_doctors; ++i) {
                    // Lewati dokter jika tidak tersedia untuk shift ini
                    if (!is_doctor_available(doctors, i, day, shift)) continue;
                    
                    // Lewati dokter jika sudah ditugaskan untuk shift ini
                    if (is_doctor_already_assigned(day, shift, i)) continue;

                    // Hitung skor dokter untuk shift ini
                    int score = calculate_score(doctors, i, day, shift);
                    // Tambahkan sedikit randomness untuk menghindari bias jika skor sama
                    score += rand() % 3;

                    // Pilih dokter dengan skor tertinggi
                    if (score > best_score) {
                        best_score = score;
                        best_doctor = i;
                    }
                }

                // Jika dokter ditemukan, tugaskan shift kepadanya
                if (best_doctor >= 0) {
                    current_schedule.doctor_ids[current_schedule.num_doctors] = best_doctor;
                    current_schedule.num_doctors++;
                    assign_shift(doctors, day, shift, best_doctor);
                } else {
                    break; // Berhenti mencari slot tambahan jika tidak ada dokter
                }
            }
            
            // Simpan schedule
            schedule[schedule_count++] = current_schedule;
        }
    }
}

// Fungsi untuk mengecek apakah dokter sudah ditugaskan untuk shift tertentu
int is_doctor_already_assigned(int day, int shift, int doctor_id) {
    for (int i = 0; i < schedule_count; i++) {
        if (schedule[i].day == day && schedule[i].shift == shift) {
            for (int j = 0; j < schedule[i].num_doctors; j++) {
                if (schedule[i].doctor_ids[j] == doctor_id) {
                    return 1;
                }
            }
            break;
        }
    }
    return 0;
}

// Fungsi untuk menghitung skor dokter untuk shift tertentu
int calculate_score(Doctor* doctors, int id, int day, int shift) {
    int score = 0;

    // Poin untuk preferensi shift
    if (doctors[id].preferred_shift == shift) {
        score += 20;
    } else {
        score -= 8; // Penalti jika bukan shift yang disukai
    }

    // Poin untuk preferensi waktu bulan
    if (doctors[id].preferred_time == WAKTU_CAMPUR) {
        score += 5;
    } else if (doctors[id].preferred_time == WAKTU_AWAL_BULAN && is_early_month(day)) {
        score += 15;
    } else if (doctors[id].preferred_time == WAKTU_AKHIR_BULAN && !is_early_month(day)) {
        score += 15;
    } else {
        score -= 10; // Penalti jika bukan waktu bulan yang disukai
    }

    // Poin untuk meratakan beban kerja (dokter dengan shift lebih sedikit lebih disukai)
    score += (20 - doctors[id].total_shifts_assigned);
    
    // Penalti untuk pelanggaran preferensi sebelumnya
    score -= doctors[id].violation_count * 2;

    // Poin untuk meratakan beban kerja mingguan
    int week = get_week_number(day);
    score += (doctors[id].max_shifts_per_week - doctors[id].weekly_shifts[week]) * 3;

    // Poin bonus berdasarkan tingkat dokter
    score += (int)doctors[id].tingkat * 5;

    // Poin bonus tambahan untuk shift malam berdasarkan tingkat dokter
    if (shift == SHIFT_MALAM) {
        score += (int)doctors[id].tingkat * 4;
    }

    return score;
}

// Fungsi untuk menugaskan shift kepada dokter
void assign_shift(Doctor* doctors, int day, int shift, int doctor_id) {
    int week = get_week_number(day);

    // Perbarui statistik dokter
    doctors[doctor_id].total_shifts_assigned++;
    doctors[doctor_id].weekly_shifts[week]++;
    doctors[doctor_id].shift_count[shift]++;

    // Hitung pelanggaran preferensi
    int violation = 0;
    if (doctors[doctor_id].preferred_shift != shift) violation = 1;
    if (doctors[doctor_id].preferred_time == WAKTU_AWAL_BULAN && !is_early_month(day)) violation = 1;
    if (doctors[doctor_id].preferred_time == WAKTU_AKHIR_BULAN && is_early_month(day)) violation = 1;
    if (doctors[doctor_id].weekly_shifts[week] > doctors[doctor_id].max_shifts_per_week) violation = 1;

    doctors[doctor_id].violation_count += violation;
}

// Fungsi untuk mengecek ketersediaan dokter untuk shift tertentu
int is_doctor_available(Doctor* doctors, int id, int day, int shift) {
    // Periksa apakah Koass diizinkan untuk shift malam
    if (doctors[id].tingkat == TINGKAT_KOASS && shift == SHIFT_MALAM) {
        return 0;
    }

    // Periksa apakah beban kerja mingguan sudah melebihi batas
    int week = get_week_number(day);
    if (doctors[id].weekly_shifts[week] >= doctors[id].max_shifts_per_week) {
        return 0;
    }

    return 1;
}

// Fungsi pembantu: mendapatkan nomor minggu dari hari (0-indexed)
int get_week_number(int day) {
    return day / 7;
}

// Fungsi pembantu: mengecek apakah hari berada di awal bulan (sebelum hari ke-15)
int is_early_month(int day) {
    return day < 15;
}

// Fungsi parsing untuk membaca preferensi shift dari string CSV
ShiftType parse_shift(const char* s) {
    if (strstr(s, "Pagi")) return SHIFT_PAGI;
    if (strstr(s, "Siang")) return SHIFT_SIANG;
    if (strstr(s, "Malam")) return SHIFT_MALAM;
    return SHIFT_PAGI;
}

// Fungsi parsing untuk membaca preferensi waktu dari string CSV
PreferensiWaktu parse_waktu(const char* s) {
    if (strstr(s, "Awal")) return WAKTU_AWAL_BULAN;
    if (strstr(s, "Akhir")) return WAKTU_AKHIR_BULAN;
    return WAKTU_CAMPUR;
}

// Fungsi untuk menyimpan jadwal ke file CSV di folder data
void save_schedule_to_csv(Doctor* doctors, int num_doctors) {
    FILE* f = fopen("../data/jadwal_dokter.csv", "w");
    if (!f) {
        printf("ERROR: Gagal membuat file '../data/jadwal_dokter.csv'\n");
        return;
    }
    
    fprintf(f, "Tanggal,Hari,Shift,Nama_Dokter,Bidang,Tingkat\n");
    const char* hari[] = {"Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu", "Minggu"};
    const char* tingkat_str[] = {"Koass", "Residen", "Spesialis", "Konsulen"};
    const char* shift_names[] = {"Pagi", "Siang", "Malam"};

    for (int i = 0; i < schedule_count; i++) {
        int d = schedule[i].day;
        int s = schedule[i].shift;
        
        // Jika tidak ada dokter untuk shift ini
        if (schedule[i].num_doctors == 0) {
            fprintf(f, "%d,%s,%s,TIDAK_ADA,TIDAK_ADA,TIDAK_ADA\n",
                    d + 1, hari[d % 7], shift_names[s]);
        } else {
            // Tulis setiap dokter yang bertugas
            for (int j = 0; j < schedule[i].num_doctors; j++) {
                int id = schedule[i].doctor_ids[j];
                fprintf(f, "%d,%s,%s,%s,%s,%s\n",
                        d + 1,
                        hari[d % 7],
                        shift_names[s],
                        doctors[id].name,
                        doctors[id].bidang,
                        tingkat_str[doctors[id].tingkat]);
            }
        }
    }

    fclose(f);
    printf("Jadwal berhasil disimpan ke '../data/jadwal_dokter.csv'\n");
}