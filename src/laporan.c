#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ENTRIES 1000
#define MAX_NAME 100

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

ShiftEntry shift_data[MAX_ENTRIES];
DoctorPref doctor_data[MAX_ENTRIES];
int total_shift_entries = 0;
int total_doctor_entries = 0;

// Statistik
int total_shift[MAX_ENTRIES] = {0};
int shift_mingguan[MAX_ENTRIES][5] = {0};
int shift_count_per_type[MAX_ENTRIES][3] = {0}; // Pagi, Siang, Malam
char pelanggaran[MAX_ENTRIES][256];

int get_shift_index(const char *shift) {
    if (strcmp(shift, "Pagi") == 0) return 0;
    if (strcmp(shift, "Siang") == 0) return 1;
    if (strcmp(shift, "Malam") == 0) return 2;
    return -1;
}

void load_shift_data(const char *filename) {
    FILE *fp = fopen(filename, "r");
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
    FILE *fp = fopen(filename, "r");
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
        return tanggal >= 1 && tanggal <= 10;
    if (strcmp(pref, "Akhir") == 0 || strcmp(pref, "AkhirBulan") == 0)
        return tanggal >= 21 && tanggal <= 31;
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

        // Cek pelanggaran
        DoctorPref *pref = &doctor_data[idx];
        if (shift_mingguan[idx][minggu] > pref->max_shift_per_minggu &&
            strstr(pelanggaran[idx], "Melebihi shift") == NULL) {
            strcat(pelanggaran[idx], "Melebihi shift;");
        }

        if (strcmp(e->shift, pref->preferensi_shift) != 0 &&
            strstr(pelanggaran[idx], "Shift tidak sesuai") == NULL) {
            strcat(pelanggaran[idx], "Shift tidak sesuai;");
        }

        if (!is_waktu_sesuai(pref->preferensi_waktu, e->tanggal) &&
            strstr(pelanggaran[idx], "Waktu tidak sesuai") == NULL) {
            strcat(pelanggaran[idx], "Waktu tidak sesuai;");
        }
    }
}

void tulis_laporan(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { printf("Gagal menulis file laporan.\n"); exit(1); }

    fprintf(fp, "Nama,Bidang,Tingkat,Total_Shift,Max_Per_Minggu,Pref_Shift,Pref_Waktu,Pelanggaran,Minggu_1,Minggu_2,Minggu_3,Minggu_4,Minggu_5,Shift_Pagi,Shift_Siang,Shift_Malam\n");

    for (int i = 0; i < total_doctor_entries; i++) {
        fprintf(fp, "%s,%s,%s,%d,%d,%s,%s,%s",
                doctor_data[i].nama,
                doctor_data[i].bidang,
                doctor_data[i].tingkat,
                total_shift[i],
                doctor_data[i].max_shift_per_minggu,
                doctor_data[i].preferensi_shift,
                doctor_data[i].preferensi_waktu,
                strlen(pelanggaran[i]) > 0 ? pelanggaran[i] : "-");

        for (int j = 0; j < 5; j++)
            fprintf(fp, ",%d", shift_mingguan[i][j]);

        for (int j = 0; j < 3; j++)
            fprintf(fp, ",%d", shift_count_per_type[i][j]);

        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("Laporan berhasil disimpan ke %s\n", filename);
}
/*
int main() {
    load_shift_data("jadwal_dokter.csv");
    load_doctor_data("daftar_dokter.csv");

    proses_data();
    tulis_laporan("laporan_dokter.csv");

    return 0;
}
*/