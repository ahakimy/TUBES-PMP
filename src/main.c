#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Untuk strlen, strcmp, dll.
#include <ctype.h>  // Untuk tolower

// Include header dari semua modul
#include "dokter.h"       // Untuk manajemen dokter
#include "penjadwalan.h"  // Untuk penjadwalan otomatis
#include "laporan.h"      // Untuk laporan kinerja dokter
#include "laporan1.h"     // Untuk melihat jadwal (harian, mingguan, bulanan)

// Definisikan path file CSV
#define DOKTER_CSV_PATH "data/daftar_dokter.csv"
#define JADWAL_CSV_PATH "data/jadwal_dokter.csv"
#define LAPORAN_CSV_PATH "data/laporan_dokter.csv"

// Fungsi untuk membersihkan buffer input (penting setelah scanf %d)
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Menu Manajemen Dokter
void menu_manajemen_dokter() {
    int pilihan;
    do {
        printf("\n=== MENU MANAJEMEN DOKTER ===\n");
        printf("1. Tambah Dokter Baru\n");
        printf("2. Hapus Dokter\n");
        printf("3. Cari Dokter\n");
        printf("4. Tampilkan Semua Dokter\n");
        printf("5. Batalkan Aktivitas Terakhir (Undo)\n");
        printf("6. Tampilkan Log Aktivitas\n");
        printf("0. Kembali ke Menu Utama\n");
        printf("Pilihan: ");
        if (scanf("%d", &pilihan) != 1) {
            printf("Input tidak valid. Harap masukkan angka.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer(); // Bersihkan buffer setelah scanf

        switch (pilihan) {
            case 1: tambah_dokter_manual(); break;
            case 2: hapus_dokter(); break;
            case 3: cari_dokter_menu(); break;
            case 4: tampilkan_semua(); break;
            case 5: hapus_aktivitas_terakhir(); break;
            case 6: tampilkan_log(); break;
            case 0: printf("Kembali ke menu utama...\n"); break;
            default: printf("Pilihan tidak valid.\n");
        }
    } while (pilihan != 0);
}

// Menu Penjadwalan Otomatis
void menu_penjadwalan_otomatis() {
    printf("\n=== MENU PENJADWALAN OTOMATIS ===\n");
    printf("Memuat data dokter dari %s...\n", DOKTER_CSV_PATH);
    if (!load_doctors_from_csv(DOKTER_CSV_PATH)) {
        printf("Gagal memuat data dokter atau file kosong. Pastikan %s ada dan berisi data.\n", DOKTER_CSV_PATH);
        return;
    }
    printf("Berhasil memuat %d dokter.\n", num_doctors);

    printf("Menghasilkan jadwal bulanan...\n");
    generate_schedule();
    printf("Jadwal berhasil dibuat.\n");

    printf("Menyimpan jadwal ke %s...\n", JADWAL_CSV_PATH);
    save_schedule();
    printf("Penjadwalan selesai. Jadwal baru telah disimpan.\n");
}

// Menu Lihat Jadwal
void menu_lihat_jadwal() {
    FILE *file_jadwal = fopen(JADWAL_CSV_PATH, "r");
    if (file_jadwal == NULL) {
        printf("Error: Tidak dapat membuka file jadwal %s. Harap lakukan penjadwalan terlebih dahulu.\n", JADWAL_CSV_PATH);
        return;
    }

    int pilihan;
    do {
        printf("\n=== MENU LIHAT JADWAL ===\n");
        printf("1. Tampilkan Jadwal Harian\n");
        printf("2. Tampilkan Jadwal Mingguan\n");
        printf("3. Tampilkan Jadwal Bulanan\n");
        printf("0. Kembali ke Menu Utama\n");
        printf("Pilihan: ");
        if (scanf("%d", &pilihan) != 1) {
            printf("Input tidak valid. Harap masukkan angka.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer(); // Bersihkan buffer setelah scanf

        switch (pilihan) {
            case 1: jalankanOpsi1(file_jadwal); break;
            case 2: jalankanOpsi2(file_jadwal); break;
            case 3: jalankanOpsi3(file_jadwal); break;
            case 0: printf("Kembali ke menu utama...\n"); break;
            default: printf("Pilihan tidak valid.\n");
        }
        rewind(file_jadwal); // Kembalikan pointer file ke awal untuk baca ulang
    } while (pilihan != 0);

    fclose(file_jadwal);
}

void menu_laporan_kinerja() {
    load_shift_data(JADWAL_CSV_PATH);
    load_doctor_data(DOKTER_CSV_PATH);
    if (total_shift_entries == 0 || total_doctor_entries == 0) {
        printf("Tidak ada data yang cukup untuk membuat laporan. Pastikan file jadwal dan dokter terisi.\n");
        return;
    }

    printf("\n=== MENU LAPORAN KINERJA DOKTER --->\n");
    printf("Memproses data untuk laporan...\n");
    proses_data();
    printf("Data selesai diproses.\n");
    
    printf("Menyimpan laporan ke %s...\n", LAPORAN_CSV_PATH);
    tulis_laporan(LAPORAN_CSV_PATH);
    printf("Laporan kinerja dokter telah dibuat.\n");

    int pilihan;
    do {
        printf("\n=== SUBMENU LAPORAN KINERJA ==>\n\n");
        printf("1. Lihat Total Shift Dokter\n");
        printf("2. Lihat Pelanggaran Jadwal\n");
        printf("3. Lihat Jumlah Pelanggaran Semua Dokter\n"); // Baru
        printf("4. Lihat Pelanggaran Dokter Tertentu\n"); // Baru
        printf("0. Back\n");
        printf("Pilihan: ");
        if (scanf("%d", &pilihan) != 1) {
            printf("Input tidak valid. Harap masukkan angka.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        switch (pilihan) {
            case 1: lihat_jumlah_shift(); break;
            case 2: lihat_pelanggaran(); break;
            case 3: lihat_jumlah_pelanggaran_per_dokter(); break; // Baru
            case 4: lihat_pelanggaran_dokter(); break; // Baru
            case 0: printf("Kembali ke menu utama...\n"); break;
            default: printf("Pilihan tidak valid.\n");
        }
    } while (pilihan != 0);
}


#include "gui.h"

int main(int argc, char **argv) {
    return gui_main(argc, argv);
}

/*
// Fungsi utama
int main() {
    int pilihan;

    // Inisialisasi awal: Muat data dokter saat program dimulai
    // Output file dokter didefinisikan di dokter.h (OUTPUT_FILE_DOKTER)
    // Asumsi itu sama dengan DOKTER_CSV_PATH
    load_data_dari_csv(DOKTER_CSV_PATH);
    printf("Selamat datang di Sistem Manajemen Jadwal Dokter!\n");

    while (1) {
        printf("\n=== MENU UTAMA ===\n");
        printf("1. Manajemen Dokter\n");
        printf("2. Penjadwalan Otomatis\n");
        printf("3. Lihat Jadwal Dokter\n");
        printf("4. Laporan Kinerja Dokter\n");
        printf("0. Keluar Program\n");
        printf("Pilihan: ");
        if (scanf("%d", &pilihan) != 1) {
            printf("Input tidak valid. Harap masukkan angka.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer(); // Bersihkan buffer setelah scanf

        switch (pilihan) {
            case 1: menu_manajemen_dokter(); break;
            case 2: menu_penjadwalan_otomatis(); break;
            case 3: menu_lihat_jadwal(); break;
            case 4: menu_laporan_kinerja(); break;
            case 0:
                printf("Menyimpan data dan membersihkan memori...\n");
                save_data_to_csv(DOKTER_CSV_PATH); // Pastikan data dokter terakhir disimpan
                free_memory(); // Bebaskan semua memori linked list
                printf("Terima kasih telah menggunakan program ini.\n");
                return 0;
            default: printf("Pilihan tidak valid. Silakan pilih 0-4.\n");
        }
    }

    return 0;
}
*/