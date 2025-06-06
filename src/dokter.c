
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dokter.h" // Meng-include header dokter.h


#define LEN 100

typedef struct Dokter {
    char nama[LEN];
    char bidang[LEN];
    char tingkat[LEN];
    int max_shift_per_minggu;
    char preferensi_shift[LEN];
    char preferensi_waktu[LEN];
    struct Dokter* next;
} Dokter;

Dokter* head = NULL;

void tambah_dokter_manual() {
    Dokter* d = malloc(sizeof(Dokter));
    if (!d) return;

    printf("Nama: "); fgets(d->nama, LEN, stdin); strtok(d->nama, "\n");
    printf("Bidang: "); fgets(d->bidang, LEN, stdin); strtok(d->bidang, "\n");
    printf("Tingkat: "); fgets(d->tingkat, LEN, stdin); strtok(d->tingkat, "\n");
    printf("Max shift/minggu: "); scanf("%d", &d->max_shift_per_minggu); getchar();
    printf("Preferensi Shift (Pagi/Siang/Malam): "); fgets(d->preferensi_shift, LEN, stdin); strtok(d->preferensi_shift, "\n");
    printf("Preferensi Waktu (Campur/Awal Bulan/Akhir Bulan): "); fgets(d->preferensi_waktu, LEN, stdin); strtok(d->preferensi_waktu, "\n");

    d->next = head;
    head = d;
}

void hapus_dokter() {
    char target[LEN];
    printf("Masukkan nama dokter yang ingin dihapus: ");
    fgets(target, LEN, stdin); strtok(target, "\n");

    Dokter *curr = head, *prev = NULL;
    while (curr) {
        if (strcmp(curr->nama, target) == 0) {
            if (prev) prev->next = curr->next;
            else head = curr->next;
            free(curr);
            printf("Dokter berhasil dihapus.\n");
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    printf("Dokter tidak ditemukan.\n");
}

void tampilkan_dokter() {
    Dokter* d = head;
    printf("\n=== DAFTAR DOKTER ===\n");
    while (d) {
        printf("Nama: %s\n", d->nama);
        printf("Bidang: %s\n", d->bidang);
        printf("Tingkat: %s\n", d->tingkat);
        printf("Max shift/minggu: %d\n", d->max_shift_per_minggu);
        printf("Preferensi Shift: %s\n", d->preferensi_shift);
        printf("Preferensi Waktu: %s\n", d->preferensi_waktu);
        printf("------------------------\n");
        d = d->next;
    }
}

void simpan_data() {
    FILE* fp = fopen("daftar_dokter.csv", "w");
    fprintf(fp, "nama,bidang,tingkat,max_shift_per_minggu,preferensi_shift,preferensi_waktu\n");
    Dokter* d = head;
    while (d) {
        fprintf(fp, "%s,%s,%s,%d,%s,%s\n",
            d->nama, d->bidang, d->tingkat,
            d->max_shift_per_minggu, d->preferensi_shift, d->preferensi_waktu);
        d = d->next;
    }
    fclose(fp);
    printf("Data disimpan ke daftar_dokter.csv\n");
}

void load_data() {
    FILE* fp = fopen("daftar_dokter.csv", "r");
    if (!fp) return;

    char line[512];
    fgets(line, sizeof(line), fp); // Skip header

    while (fgets(line, sizeof(line), fp)) {
        Dokter* d = malloc(sizeof(Dokter));
        if (!d) continue;
        sscanf(line, " %99[^,],%99[^,],%99[^,],%d,%99[^,],%99[^\n]",
            d->nama, d->bidang, d->tingkat,
            &d->max_shift_per_minggu, d->preferensi_shift, d->preferensi_waktu);
        d->next = head;
        head = d;
    }
    fclose(fp);
}

/* int main() {
    load_data();
    int pilihan;

    do {
        printf("\nMENU:\n");
        printf("1. Tampilkan Dokter\n");
        printf("2. Tambah Dokter\n");
        printf("3. Hapus Dokter\n");
        printf("4. Simpan dan Keluar\n");
        printf("Pilihan: ");
        scanf("%d", &pilihan); getchar();

        switch (pilihan) {
            case 1: tampilkan_dokter(); break;
            case 2: tambah_dokter_manual(); break;
            case 3: hapus_dokter(); break;
            case 4: simpan_data(); break;
            default: printf("Pilihan tidak valid.\n");
        }
    } while (pilihan != 4);

    return 0;
} */
