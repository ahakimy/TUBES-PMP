#include "laporan1.h"

int main() {
    FILE *file;
    int pilihan;

    // Open the CSV file
    file = fopen(nama_file, "r");
    if (file == NULL) {
        printf("Error: Tidak dapat membuka file %s\n", nama_file);
        return 1;
    }

    // Menu loop
    while (1) {
        printf("\n=== Sistem Jadwal Dokter ===\n");
        printf("1. Tampilkan Jadwal Harian\n");
        printf("2. Tampilkan Jadwal Mingguan\n");
        printf("3. Tampilkan Jadwal Bulanan\n");
        printf("4. Keluar\n");
        printf("Pilih opsi (1-4): ");
        scanf("%d", &pilihan);
        getchar(); // Clear newline from input buffer

        switch (pilihan) {
            case 1:
                jalankanOpsi1(file);
                break;
            case 2:
                jalankanOpsi2(file);
                break;
            case 3:
                jalankanOpsi3(file);
                break;
            case 4:
                fclose(file);
                printf("Terima kasih telah menggunakan program ini.\n");
                return 0;
            default:
                printf("Pilihan tidak valid. Silakan pilih 1-4.\n");
        }
    }

    return 0;
}