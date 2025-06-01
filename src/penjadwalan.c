#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Definisi konstanta untuk ukuran maksimum dan jumlah shift
#define MAX_DOCTORS 50
#define MAX_NAME_LEN 100
#define DAYS_IN_MONTH 30
#define SHIFTS_PER_DAY 3
#define MAX_DOCTORS_PER_SHIFT 5
#define TOTAL_SHIFTS (DAYS_IN_MONTH * SHIFTS_PER_DAY)

// UBAH NILAI INI UNTUK MENGATUR JUMLAH DOKTER PER SHIFT
int DOCTORS_PER_SHIFT = 2; // <-- UBAH DISINI untuk mengatur berapa dokter per shift

// Enumerasi untuk jenis shift
typedef enum {
    SHIFT_PAGI = 0,
    SHIFT_SIANG = 1,
    SHIFT_MALAM = 2
} ShiftType;

// Enumerasi untuk tingkat dokter
typedef enum {
    TINGKAT_KOASS = 0,
    TINGKAT_RESIDEN,
    TINGKAT_SPESIALIS,
    TINGKAT_KONSULEN
} TingkatDokter;

// Enumerasi untuk preferensi waktu shift
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
    int weekly_shifts[5]; // Jumlah shift per minggu (untuk 5 minggu dalam sebulan)
    int shift_count[3]; // Jumlah shift pagi, siang, malam
    int violation_count; // Jumlah pelanggaran preferensi
} Doctor;

// Struktur data untuk menyimpan entri jadwal (support multiple dokter per shift)
typedef struct {
    int day;
    int shift;
    int doctor_ids[MAX_DOCTORS_PER_SHIFT];
    int num_doctors;
} Schedule;

// Deklarasi variabel global
Doctor doctors[MAX_DOCTORS];
int num_doctors = 0;
Schedule schedule[TOTAL_SHIFTS];
int schedule_count = 0;

// Deklarasi fungsi-fungsi
int load_doctors_from_csv(const char* filename);
void generate_schedule();
void save_schedule();
void save_report();
int get_week_number(int day);
int is_early_month(int day);
int is_doctor_available(int id, int day, int shift);
int calculate_score(int id, int day, int shift);
void assign_shift(int day, int shift, int doctor_id);
int is_doctor_already_assigned(int day, int shift, int doctor_id);

// Fungsi utama program
int main() {
    // Memuat data dokter dari file CSV
    if (!load_doctors_from_csv("daftar_dokter.csv")) {
        printf("ERROR: Gagal membuka file 'daftar_dokter.csv'.\n");
        return 1;
    }
    
    // Initialize random seed
    srand((unsigned)time(NULL));
    
    // Menghasilkan jadwal otomatis
    generate_schedule();
    
    // Menyimpan jadwal ke file CSV
    save_schedule();
    
    // Menyimpan laporan ke file CSV
    save_report();
    
    return 0;
}

// Fungsi untuk menghasilkan jadwal otomatis
void generate_schedule() {
    schedule_count = 0;

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
                    if (!is_doctor_available(i, day, shift)) continue;
                    
                    // Lewati dokter jika sudah ditugaskan untuk shift ini
                    if (is_doctor_already_assigned(day, shift, i)) continue;

                    // Hitung skor dokter untuk shift ini
                    int score = calculate_score(i, day, shift);
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
                    assign_shift(day, shift, best_doctor);
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
int calculate_score(int id, int day, int shift) {
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
void assign_shift(int day, int shift, int doctor_id) {
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

// Fungsi untuk menyimpan jadwal ke file CSV
void save_schedule() {
    FILE* f = fopen("jadwal_dokter.csv", "w");
    if (!f) {
        printf("ERROR: Gagal membuat file 'jadwal_dokter.csv'\n");
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
}

// Fungsi untuk menyimpan laporan ke file CSV
void save_report() {
    FILE* f = fopen("laporan_dokter.csv", "w");
    if (!f) {
        printf("ERROR: Gagal membuat file 'laporan_dokter.csv'\n");
        return;
    }
    
    fprintf(f, "Nama,Bidang,Tingkat,Total_Shift,Max_Per_Minggu,Pref_Shift,Pref_Waktu,Pelanggaran,");
    fprintf(f, "Minggu_1,Minggu_2,Minggu_3,Minggu_4,Minggu_5,");
    fprintf(f, "Shift_Pagi,Shift_Siang,Shift_Malam\n");

    const char* shift_pref_str[] = {"Pagi", "Siang", "Malam"};
    const char* waktu_pref_str[] = {"Awal Bulan", "Akhir Bulan", "Campur"};
    const char* tingkat_str[] = {"Koass", "Residen", "Spesialis", "Konsulen"};

    for (int i = 0; i < num_doctors; i++) {
        Doctor* d = &doctors[i];
        fprintf(f, "%s,%s,%s,%d,%d,%s,%s,%d,",
                d->name,
                d->bidang,
                tingkat_str[d->tingkat],
                d->total_shifts_assigned,
                d->max_shifts_per_week,
                shift_pref_str[d->preferred_shift],
                waktu_pref_str[d->preferred_time],
                d->violation_count);
        for (int j = 0; j < 5; j++) fprintf(f, "%d,", d->weekly_shifts[j]);
        fprintf(f, "%d,%d,%d\n", d->shift_count[0], d->shift_count[1], d->shift_count[2]);
    }

    fclose(f);
}

// Fungsi pembantu: mendapatkan nomor minggu dari hari (0-indexed)
int get_week_number(int day) {
    return day / 7;
}

// Fungsi pembantu: mengecek apakah hari berada di awal bulan (sebelum hari ke-15)
int is_early_month(int day) {
    return day < 15;
}

// Fungsi untuk mengecek ketersediaan dokter untuk shift tertentu
int is_doctor_available(int id, int day, int shift) {
    if (id >= num_doctors) return 0;

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

// Fungsi parsing
ShiftType parse_shift(const char* s) {
    if (strstr(s, "Pagi")) return SHIFT_PAGI;
    if (strstr(s, "Siang")) return SHIFT_SIANG;
    if (strstr(s, "Malam")) return SHIFT_MALAM;
    return SHIFT_PAGI;
}

TingkatDokter parse_tingkat(const char* s) {
    if (strstr(s, "Koass")) return TINGKAT_KOASS;
    if (strstr(s, "Residen")) return TINGKAT_RESIDEN;
    if (strstr(s, "Spesialis")) return TINGKAT_SPESIALIS;
    if (strstr(s, "Konsulen")) return TINGKAT_KONSULEN;
    return TINGKAT_KOASS;
}

PreferensiWaktu parse_waktu(const char* s) {
    if (strstr(s, "Awal")) return WAKTU_AWAL_BULAN;
    if (strstr(s, "Akhir")) return WAKTU_AKHIR_BULAN;
    return WAKTU_CAMPUR;
}

// Fungsi untuk memuat data dokter dari file CSV
int load_doctors_from_csv(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    char line[512];
    // Baca baris header
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return 0;
    }
    
    // Baca setiap baris data dokter
    while (fgets(line, sizeof(line), f) && num_doctors < MAX_DOCTORS) {
        Doctor* d = &doctors[num_doctors];
        char* token;

        // Nama
        token = strtok(line, ",");
        if (token) strcpy(d->name, token); else continue;

        // Bidang
        token = strtok(NULL, ",");
        if (token) strcpy(d->bidang, token); else continue;

        // Tingkat
        token = strtok(NULL, ",");
        if (token) d->tingkat = parse_tingkat(token); else continue;

        // Max Shift Per Minggu
        token = strtok(NULL, ",");
        if (token) d->max_shifts_per_week = atoi(token); else continue;

        // Preferred Shift
        token = strtok(NULL, ",");
        if (token) d->preferred_shift = parse_shift(token); else continue;

        // Preferred Time
        token = strtok(NULL, ",\n\r");
        if (token) d->preferred_time = parse_waktu(token); else continue;

        // Inisialisasi statistik dokter
        d->total_shifts_assigned = d->violation_count = 0;
        memset(d->weekly_shifts, 0, sizeof(d->weekly_shifts));
        memset(d->shift_count, 0, sizeof(d->shift_count));
        num_doctors++;
    }

    fclose(f);
    return 1;
}