#ifndef DOKTER_H
#define DOKTER_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define LEN 100
#define MAX_DOKTER 100 // Define a constant for the maximum number of doctors
#define OUTPUT_FILE "../data/daftar_dokter.csv" // Path relatif ke executable di bin/

typedef struct Dokter {
    char nama[LEN];
    char bidang[LEN];
    char tingkat[LEN];
    int max_shift_per_minggu;
    char preferensi_shift[LEN];
    char preferensi_waktu[LEN];
    struct Dokter* next;
} Dokter;

typedef enum { AKSI_TAMBAH, AKSI_HAPUS } AksiType;

typedef struct Aktivitas {
    AksiType tipe;
    Dokter* dokterData;  // Salinan data dokter yg dihapus/tambah utk undo
    struct Aktivitas* next;
} Aktivitas;

extern Dokter* head;
extern Aktivitas* aktivitas_head;

Dokter* salin_dokter(Dokter* src);
void tambah_aktivitas(AksiType tipe, Dokter* data);
void hapus_aktivitas_terakhir();
void tampilkan_log();
void tambah_dokter_manual();
void hapus_dokter();
int compare(const Dokter* a, const Dokter* b, int mode);
void sort_dokter_list(Dokter** headRef, int mode);
int contains_keyword(const char* src, const char* keyword);
void tampilkan_hasil(Dokter* hasil);
void cari_dokter_nama();
void cari_dokter_bidang();
void cari_dokter_tingkat();
void cari_dokter_menu();
void sortir_dokter();
void statistik();
void tampilkan_semua();
void load_data_dari_csv(const char *nama_file);
void save_data_to_csv(const char *nama_file);
void menu();
void free_memory(); // Fungsi baru untuk membebaskan memori

#endif // DOKTER_H