#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "laporan.h"

#define MAX_ENTRIES 1000 //Max 
#define MAX_NAME 100     //Maksimal panjang nama
#define MAX_LINE 1024    //maksimal panjang line
#define INPUT_FILE_JADWAL "data/jadwal_dokter.csv"
#define INPUT_FILE_DOKTER "data/daftar_dokter.csv"
#define INPUT_FILE_LAPORAN "data/laporan_dokter.csv"
#define OUTPUT_FILE_LAPORAN "data/laporan_dokter.csv"

ShiftEntry shift_data[MAX_ENTRIES]; //Menyimpan data dokter yang meliputi shift(tanggal, hari.. )
DoctorPref doctor_data[MAX_ENTRIES]; //Menyimpan data dokter terkait preferensi
int total_shift_entries = 0; //Menyimpan jumlah total data shift yang berhasil dimuat dari file.
int total_doctor_entries = 0; //Menyimpan jumlah total dokter yang berhasil dimuat dari file.

//Output ke file laporan_dokter.csv
int total_shift[MAX_ENTRIES] = {0}; // total_shift tiap dokter
int shift_mingguan[MAX_ENTRIES][5] = {{0}}; //
int shift_count_per_type[MAX_ENTRIES][3] = {{0}}; 
int pelanggaran[MAX_ENTRIES] = {0}; //Jumlah pelanggaran

int get_shift_index(const char *shift) {
    if (strcmp(shift, "Pagi") == 0) return 0;
    if (strcmp(shift, "Siang") == 0) return 1;
    if (strcmp(shift, "Malam") == 0) return 2;
    return -1;
}

void load_shift_data(const char *filename) {
    FILE *fp = fopen(INPUT_FILE_JADWAL, "r");
    if (!fp) { printf("Gagal membuka file shift.\n"); exit(1); }

    char line[512];
    fgets(line, sizeof(line), fp); 

    while (fgets(line, sizeof(line), fp)) {
        ShiftEntry *e = &shift_data[total_shift_entries];
        sscanf(line, "%d,%[^,],%[^,],%[^,],%[^,],%s",
               &e->tanggal, e->hari, e->shift, e->nama, e->bidang, e->tingkat);
        total_shift_entries++;
    }

    fclose(fp);
}

void load_doctor_data(const char *filename) {
    FILE *fp = fopen(INPUT_FILE_DOKTER, "r");
    if (!fp) { printf("Gagal membuka file dokter.\n"); exit(1); }

    char line[512];
    fgets(line, sizeof(line), fp); 

    while (fgets(line, sizeof(line), fp)) {
        DoctorPref *d = &doctor_data[total_doctor_entries];
        sscanf(line, "%[^,],%[^,],%[^,],%d,%[^,],%s",
               d->nama, d->bidang, d->tingkat,
               &d->max_shift_per_minggu,
               d->preferensi_shift, d->preferensi_waktu);
        total_doctor_entries++;
    }

    fclose(fp);
}

int is_waktu_sesuai(const char *pref, int tanggal) {
    if (strcmp(pref, "Campur") == 0) return 1;
    if (strcmp(pref, "Awal") == 0 || strcmp(pref, "AwalBulan") == 0)
        return tanggal >= 1 && tanggal <= 15;
    if (strcmp(pref, "Akhir") == 0 || strcmp(pref, "AkhirBulan") == 0)
        return tanggal >= 16 && tanggal <= 31;
    return 1;
}

int find_doctor_index(const char *nama) {
    for (int i = 0; i < total_doctor_entries; i++) {
        if (strcmp(doctor_data[i].nama, nama) == 0)
            return i;
    }
    return -1;
}

void proses_data() {
    for (int i = 0; i < total_shift_entries; i++) {
        ShiftEntry *e = &shift_data[i];
        int idx = find_doctor_index(e->nama);
        if (idx == -1) continue;

        int minggu = (e->tanggal - 1) / 7;
        int shift_idx = get_shift_index(e->shift);

        total_shift[idx]++;
        shift_mingguan[idx][minggu]++;
        if (shift_idx != -1) shift_count_per_type[idx][shift_idx]++;

        // Cek pelanggaran dan langsung tambahkan hitungan
        DoctorPref *pref = &doctor_data[idx];

        if (shift_mingguan[idx][minggu] > pref->max_shift_per_minggu) {
            pelanggaran[idx]++;
        }

        if (strcmp(e->shift, pref->preferensi_shift) != 0) {
            pelanggaran[idx]++;
        }

        if (!is_waktu_sesuai(pref->preferensi_waktu, e->tanggal)) {
            pelanggaran[idx]++;
        }
    }
}


void tulis_laporan(const char *filename) {
    FILE *fp = fopen(OUTPUT_FILE_LAPORAN, "w");
    if (!fp) { printf("Gagal menulis file laporan.\n"); exit(1); }

    fprintf(fp, "Nama,Bidang,Tingkat,Total_Shift,Max_Per_Minggu,Pref_Shift,Pref_Waktu,Pelanggaran,Minggu_1,Minggu_2,Minggu_3,Minggu_4,Minggu_5,Shift_Pagi,Shift_Siang,Shift_Malam\n");

    for (int i = 0; i < total_doctor_entries; i++) {
        fprintf(fp, "%s,%s,%s,%d,%d,%s,%s,%d",
                doctor_data[i].nama,
                doctor_data[i].bidang,
                doctor_data[i].tingkat,
                total_shift[i],
                doctor_data[i].max_shift_per_minggu,
                doctor_data[i].preferensi_shift,
                doctor_data[i].preferensi_waktu,
                pelanggaran[i]);

        for (int j = 0; j < 5; j++)
            fprintf(fp, ",%d", shift_mingguan[i][j]);

        for (int j = 0; j < 3; j++)
            fprintf(fp, ",%d", shift_count_per_type[i][j]);

        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("Laporan berhasil disimpan ke %s\n", filename);
}

void lihat_jumlah_shift() {
    char nama_dokter[100];
    printf("Masukkan nama dokter: ");
    fgets(nama_dokter, sizeof(nama_dokter), stdin);
    nama_dokter[strcspn(nama_dokter, "\n")] = 0; // hapus newline

    FILE *fp = fopen(INPUT_FILE_LAPORAN, "r");
    if (!fp) {
        printf("Gagal membuka laporan_dokter.csv\n");
        return;
    }

    char line[MAX_LINE];
    fgets(line, sizeof(line), fp);

    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, ",");
        if (token && strcmp(token, nama_dokter) == 0) {
            found = 1;
            for (int i = 0; i < 2; i++) strtok(NULL, ",");
            char *total_shift = strtok(NULL, ",");
            printf("Total shift untuk %s: %s\n", nama_dokter, total_shift);
            break;
        }
    }

    if (!found) {
        printf("Dokter dengan nama '%s' tidak ditemukan.\n", nama_dokter);
    }
    fclose(fp);
}

void lihat_pelanggaran() {
    int minggu_saat_ini = 0;
    int tanggal_ke_minggu[32] = {0};

    for (int i = 0; i < total_shift_entries; i++) {
        ShiftEntry *e = &shift_data[i];
        if (strcmp(e->hari, "Senin") == 0 && tanggal_ke_minggu[e->tanggal] == 0) {
            minggu_saat_ini++;
        }
        tanggal_ke_minggu[e->tanggal] = minggu_saat_ini;
    }

    int temp_shift_mingguan[MAX_ENTRIES][5] = {0};

    for (int i = 0; i < total_shift_entries; i++) {
        ShiftEntry *e = &shift_data[i];
        int idx = find_doctor_index(e->nama);
        if (idx == -1) continue;

        DoctorPref *pref = &doctor_data[idx];
        int minggu = tanggal_ke_minggu[e->tanggal] - 1;
        temp_shift_mingguan[idx][minggu]++;

        // Cek preferensi shift
        if (strcmp(e->shift, pref->preferensi_shift) != 0) {
            printf("[%d] %s - Shift tidak sesuai preferensi (jadwal: %s, preferensi: %s)\n",
                   e->tanggal, e->nama, e->shift, pref->preferensi_shift);
        }

        // Cek preferensi waktu
        if (!is_waktu_sesuai(pref->preferensi_waktu, e->tanggal)) {
            printf("[%d] %s - Waktu tidak sesuai preferensi (jadwal tanggal: %d, preferensi: %s bulan)\n",
                   e->tanggal, e->nama, e->tanggal, pref->preferensi_waktu);
        }

        // Cek kelebihan shift mingguan
        if (temp_shift_mingguan[idx][minggu] > pref->max_shift_per_minggu) {
            printf("[%d] %s - Melebihi shift mingguan maksimum (%d per minggu)\n",
                   e->tanggal, e->nama, pref->max_shift_per_minggu);
        }
    }
}
