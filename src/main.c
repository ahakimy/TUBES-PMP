#include <stdio.h>
#include "../include/penjadwalan.h"
#include "../include/dokter.h"

int main() {
    // Load data dokter awal dari file CSV untuk penjadwalan
    if (!load_doctors_from_csv(INPUT_FILE)) {
        fprintf(stderr, "Gagal memuat data dokter dari %s. Memulai dengan data kosong.\n", INPUT_FILE);
    } else {
        printf("Data dokter berhasil dimuat dari %s.\n", INPUT_FILE);
    }

    // Load data dokter dari file CSV untuk modul dokter (opsional, jika ingin sinkronisasi)
    load_data_dari_csv(OUTPUT_FILE);

    int pilihan;
    do {
        printf("\n=== MENU UTAMA ===\n");
        printf("1. Tambah dokter\n");
        printf("2. Hapus dokter\n");
        printf("3. Cari dokter\n");
        printf("4. Sortir dokter\n");
        printf("5. Tampilkan semua dokter\n");
        printf("6. Statistik dokter\n");
        printf("7. Undo aktivitas terakhir\n");
        printf("8. Tampilkan log aktivitas\n");
        printf("9. Generate dan simpan jadwal\n");
        printf("0. Keluar\n");
        printf("Pilihan: ");
        scanf("%d", &pilihan); getchar();

        switch (pilihan) {
            case 1: tambah_dokter_manual(); break;
            case 2: hapus_dokter(); break;
            case 3: cari_dokter_menu(); break;
            case 4: sortir_dokter(); break;
            case 5: tampilkan_semua(); break;
            case 6: statistik(); break;
            case 7: hapus_aktivitas_terakhir(); break;
            case 8: tampilkan_log(); break;
            case 9:
                generate_schedule();
                save_schedule();
                printf("Jadwal berhasil dibuat dan disimpan ke %s.\n", OUTPUT_FILE);
                break;
            case 0:
                printf("Menyimpan data sebelum keluar...\n");
                save_data_to_csv(OUTPUT_FILE);
                free_memory();
                printf("Keluar program...\n");
                break;
            default: printf("Pilihan tidak valid.\n");
        }
    } while (pilihan != 0);

    return 0;
}