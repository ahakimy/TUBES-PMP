#include "dokter.h"
#include <errno.h>    // Untuk strerror dan errno
#include <sys/stat.h> // Untuk mkdir
#include <string.h>   // Untuk strcspn, strncpy, strcmp, strstr
#include <stdlib.h>   // Untuk malloc, free, atoi
#include <stdio.h>    // Untuk FILE, fopen, fclose, printf, fgets, scanf, getchar
#include <ctype.h>    // Untuk tolower, strcasecmp

Dokter* head = NULL;
Aktivitas* aktivitas_head = NULL;

// Fungsi bantu membuat salinan dokter
Dokter* salin_dokter(Dokter* src) {
    if (!src) return NULL;
    Dokter* d = malloc(sizeof(Dokter));
    if (!d) return NULL;
    // Salin semua anggota struktur secara byte-per-byte
    memcpy(d, src, sizeof(Dokter));
    d->next = NULL; // Pastikan next dari salinan adalah NULL
    return d;
}

// --- MANAGE AKTIVITAS (UNDO) ---
void tambah_aktivitas(AksiType tipe, Dokter* data) {
    Aktivitas* a = malloc(sizeof(Aktivitas));
    if (!a) {
        free(data); // Jika data adalah salinan dan malloc aktivitas gagal, bebaskan data
        return;
    }
    a->tipe = tipe;
    a->dokterData = data; // Gunakan data langsung, karena sudah disalin di tempat pemanggil
    if (!a->dokterData) { // Cek lagi jika salin_dokter gagal
        free(a);
        return;
    }
    a->next = aktivitas_head;
    aktivitas_head = a;
}

void hapus_aktivitas_terakhir() {
    if (!aktivitas_head) {
        printf("Tidak ada aktivitas untuk dibatalkan.\n");
        return;
    }
    Aktivitas* a = aktivitas_head;
    aktivitas_head = aktivitas_head->next;

    if (a->tipe == AKSI_TAMBAH) {
        Dokter *curr = head, *prev = NULL;
        int found = 0;
        while (curr) {
            // Bandingkan nama dokter untuk mencari yang ditambahkan
            if (strcmp(curr->nama, a->dokterData->nama) == 0 &&
                strcmp(curr->bidang, a->dokterData->bidang) == 0) { // Tambah bidang untuk identifikasi unik
                if (prev) prev->next = curr->next;
                else head = curr->next;
                printf("Undo: Penambahan dokter %s dibatalkan.\n", a->dokterData->nama);
                free(curr); // Bebaskan memori dokter yang dihapus dari list
                found = 1;
                break;
            }
            prev = curr;
            curr = curr->next;
        }
        if (!found) {
            printf("Undo: Dokter %s tidak ditemukan dalam daftar aktif (mungkin sudah dihapus/diedit).\n", a->dokterData->nama);
        }
    } else if (a->tipe == AKSI_HAPUS) {
        // Dokter yang dihapus akan dikembalikan ke head
        a->dokterData->next = head;
        head = a->dokterData; // DokterData sekarang menjadi bagian dari linked list utama
        printf("Undo: Penghapusan dokter %s dibatalkan.\n", a->dokterData->nama);
    }
    
    // bebaskan aktivitas node itu sendiri, bukan dokterData karena dokterData bisa sudah dipakai atau disalin
    free(a); 
    save_data_to_csv(OUTPUT_FILE_DOKTER); // Simpan setelah undo
}


// --- LOG AKTIVITAS (Untuk tampilkan) ---
void tampilkan_log() {
    if (!aktivitas_head) {
        printf("Belum ada aktivitas.\n");
        return;
    }

    Aktivitas* curr = aktivitas_head;
    int i = 1;
    printf("\n=== LOG AKTIVITAS (terbaru di atas) ===\n");
    while (curr) {
        if (curr->dokterData) { // Pastikan dokterData tidak NULL
            printf("%d. %s dokter: %s\n", i++,
                   curr->tipe == AKSI_TAMBAH ? "Tambah" : "Hapus",
                   curr->dokterData->nama);
        } else {
            printf("%d. Aktivitas tidak diketahui (data dokter hilang)\n", i++);
        }
        curr = curr->next;
    }
}

// Fungsi bantu untuk membersihkan newline dari fgets
static void clean_newline(char* str) {
    str[strcspn(str, "\n")] = 0;
    // Juga hapus carriage return jika ada (untuk kompatibilitas lintas OS)
    str[strcspn(str, "\r")] = 0;
}

// --- TAMBAH DOKTER ---
void tambah_dokter_manual() {
    Dokter* d = malloc(sizeof(Dokter));
    if (!d) {
        perror("Gagal alokasi memori untuk dokter baru");
        return;
    }

    printf("Nama: ");
    if (!fgets(d->nama, LEN, stdin)) { free(d); return; }
    clean_newline(d->nama);
    if (strlen(d->nama) == 0) { printf("Nama tidak boleh kosong.\n"); free(d); return; }

    printf("Bidang: ");
    if (!fgets(d->bidang, LEN, stdin)) { free(d); return; }
    clean_newline(d->bidang);
    if (strlen(d->bidang) == 0) { printf("Bidang tidak boleh kosong.\n"); free(d); return; }

    printf("Tingkat (Koass/Residen/Spesialis/Konsulen): ");
    if (!fgets(d->tingkat, LEN, stdin)) { free(d); return; }
    clean_newline(d->tingkat);
    if (strlen(d->tingkat) == 0) { printf("Tingkat tidak boleh kosong.\n"); free(d); return; }

    printf("Max shift/minggu: ");
    if (scanf("%d", &d->max_shift_per_minggu) != 1) {
        printf("Input max shift tidak valid.\n");
        while (getchar() != '\n'); // Bersihkan buffer input
        free(d); return;
    }
    getchar(); // Konsumsi newline

    printf("Preferensi Shift (Pagi/Siang/Malam): ");
    if (!fgets(d->preferensi_shift, LEN, stdin)) { free(d); return; }
    clean_newline(d->preferensi_shift);
    if (strlen(d->preferensi_shift) == 0) { printf("Preferensi Shift tidak boleh kosong.\n"); free(d); return; }

    printf("Preferensi Waktu (Campur/Awal Bulan/Akhir Bulan): ");
    if (!fgets(d->preferensi_waktu, LEN, stdin)) { free(d); return; }
    clean_newline(d->preferensi_waktu);
    if (strlen(d->preferensi_waktu) == 0) { printf("Preferensi Waktu tidak boleh kosong.\n"); free(d); return; }

    // Tambahkan dokter ke linked list
    d->next = head;
    head = d;

    // Tambahkan aktivitas untuk undo (salin dokter sebelum ditambahkan ke aktivitas)
    Dokter* copy_for_undo = salin_dokter(d);
    if (copy_for_undo) {
        tambah_aktivitas(AKSI_TAMBAH, copy_for_undo);
    } else {
        printf("Peringatan: Gagal membuat salinan dokter untuk fitur undo.\n");
    }
    
    printf("Dokter %s berhasil ditambahkan.\n", d->nama);
    save_data_to_csv(OUTPUT_FILE_DOKTER); // Simpan setelah penambahan
}

// --- HAPUS DOKTER ---
void hapus_dokter() {
    char target[LEN];
    printf("Masukkan nama dokter yang ingin dihapus: ");
    if (!fgets(target, LEN, stdin)) return;
    clean_newline(target);
    if (strlen(target) == 0) { printf("Nama dokter tidak boleh kosong.\n"); return; }

    Dokter *curr = head, *prev = NULL;
    while (curr) {
        if (strcmp(curr->nama, target) == 0) {
            char konfirmasi;
            printf("Apakah Anda yakin ingin menghapus dokter '%s'? (y/n): ", target);
            if (scanf(" %c", &konfirmasi) != 1) {
                printf("Input tidak valid. Penghapusan dibatalkan.\n");
                while (getchar() != '\n');
                return;
            }
            getchar(); // Konsumsi newline
            if (tolower(konfirmasi) != 'y') {
                printf("Penghapusan dibatalkan.\n");
                return;
            }

            if (prev) prev->next = curr->next;
            else head = curr->next;

            // Salin data dokter sebelum dibebaskan, untuk aktivitas undo
            Dokter* copy_for_undo = salin_dokter(curr);
            if (copy_for_undo) {
                tambah_aktivitas(AKSI_HAPUS, copy_for_undo);
            } else {
                printf("Peringatan: Gagal membuat salinan dokter untuk fitur undo.\n");
            }
            
            free(curr); // Bebaskan memori dokter yang dihapus dari list utama
            printf("Dokter berhasil dihapus.\n");
            save_data_to_csv(OUTPUT_FILE_DOKTER); // Simpan setelah penghapusan
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    printf("Dokter tidak ditemukan.\n");
}

// --- SORTING HELPER ---
int compare(const Dokter* a, const Dokter* b, int mode) {
    if (!a || !b) return 0; // Handle NULL pointers
    switch (mode) {
        case 1: return strcmp(a->nama, b->nama);
        case 2: return a->max_shift_per_minggu - b->max_shift_per_minggu;
        case 3: return strcmp(a->tingkat, b->tingkat);
        case 4: return strcmp(a->preferensi_shift, b->preferensi_shift);
        case 5: return strcmp(a->preferensi_waktu, b->preferensi_waktu);
        default: return 0;
    }
}

// Bubble sort linked list based on mode
void sort_dokter_list(Dokter** headRef, int mode) {
    if (!(*headRef) || !(*headRef)->next) return;
    int swapped;
    Dokter *ptr1; // Pointer untuk iterasi
    Dokter *lptr = NULL; // Pointer untuk melacak node terakhir yang sudah diurutkan

    do {
        swapped = 0;
        ptr1 = *headRef;
        Dokter *prev = NULL; // Pointer untuk melacak node sebelumnya

        while (ptr1->next != lptr) {
            if (compare(ptr1, ptr1->next, mode) > 0) {
                // Swap nodes
                Dokter* temp = ptr1->next;
                ptr1->next = temp->next;
                temp->next = ptr1;

                if (prev) {
                    prev->next = temp;
                } else {
                    *headRef = temp; // Update head jika node pertama yang diswap
                }
                swapped = 1;
                // ptr1 sekarang adalah temp (node yang sudah di swap ke depan)
                // dan prev tetap menunjuk ke node yang sama sebelum swap terjadi (yaitu, node sebelum temp)
                ptr1 = temp; // Set ptr1 ke node yang baru saja dipindahkan
            }
            // Update prev dan ptr1 untuk iterasi berikutnya
            prev = ptr1;
            ptr1 = ptr1->next;
        }
        lptr = ptr1; // Setelah satu pass, node terakhir sudah pada posisi yang benar
    } while (swapped);
    save_data_to_csv(OUTPUT_FILE_DOKTER); // Simpan setelah pengurutan
}

// Fungsi cek apakah string src mengandung keyword (case insensitive)
int contains_keyword(const char* src, const char* keyword) {
    if (!src || !keyword) return 0;
    // Gunakan alokasi dinamis atau buffer yang cukup besar jika LEN sangat kecil
    // Namun untuk LEN 100, stack array cukup aman.
    char srcLow[LEN];
    char keyLow[LEN];
    
    // Pastikan tidak overflow buffer
    strncpy(srcLow, src, LEN - 1);
    srcLow[LEN - 1] = '\0';
    strncpy(keyLow, keyword, LEN - 1);
    keyLow[LEN - 1] = '\0';

    for (int i = 0; srcLow[i]; i++) srcLow[i] = tolower(srcLow[i]);
    for (int i = 0; keyLow[i]; i++) keyLow[i] = tolower(keyLow[i]);

    return strstr(srcLow, keyLow) != NULL;
}

// Tampilkan hasil pencarian
void tampilkan_hasil(Dokter* hasil) {
    if (!hasil) {
        printf("Tidak ada data dokter yang cocok dengan kriteria pencarian.\n");
        return;
    }

    printf("\n=== HASIL PENCARIAN DOKTER ===\n");
    Dokter* d = hasil;
    while (d) {
        printf("Nama: %s\n", d->nama);
        printf("Bidang: %s\n", d->bidang);
        printf("Tingkat: %s\n", d->tingkat);
        printf("Max shift/minggu: %d\n", d->max_shift_per_minggu);
        printf("Preferensi Shift: %s\n", d->preferensi_shift);
        printf("Preferensi Waktu: %s\n", d->preferensi_waktu);
        printf("---------------------------------\n");
        d = d->next;
    }
}

// Cari dokter berdasarkan nama
void cari_dokter_nama() {
    char keyword[LEN];
    printf("Masukkan nama dokter yang ingin dicari: ");
    if (!fgets(keyword, LEN, stdin)) return;
    clean_newline(keyword);
    if (strlen(keyword) == 0) { printf("Keyword tidak boleh kosong.\n"); return; }

    Dokter* hasil = NULL;
    Dokter* d = head;

    while (d) {
        if (contains_keyword(d->nama, keyword)) {
            Dokter* nd = salin_dokter(d);
            if (nd) {
                nd->next = hasil; // Tambahkan ke depan list hasil
                hasil = nd;
            }
        }
        d = d->next;
    }

    tampilkan_hasil(hasil);

    // Bebaskan memori dari list hasil pencarian
    while (hasil) {
        Dokter* temp = hasil;
        hasil = hasil->next;
        free(temp);
    }
}

// Cari dokter berdasarkan bidang
void cari_dokter_bidang() {
    char keyword[LEN];
    printf("Masukkan bidang dokter yang ingin dicari: ");
    if (!fgets(keyword, LEN, stdin)) return;
    clean_newline(keyword);
    if (strlen(keyword) == 0) { printf("Keyword tidak boleh kosong.\n"); return; }

    Dokter* hasil = NULL;
    Dokter* d = head;

    while (d) {
        if (contains_keyword(d->bidang, keyword)) {
            Dokter* nd = salin_dokter(d);
            if (nd) {
                nd->next = hasil;
                hasil = nd;
            }
        }
        d = d->next;
    }

    tampilkan_hasil(hasil);

    while (hasil) {
        Dokter* temp = hasil;
        hasil = hasil->next;
        free(temp);
    }
}

// Cari dokter berdasarkan tingkat
void cari_dokter_tingkat() {
    char keyword[LEN];
    printf("Masukkan tingkat dokter yang ingin dicari: ");
    if (!fgets(keyword, LEN, stdin)) return;
    clean_newline(keyword);
    if (strlen(keyword) == 0) { printf("Keyword tidak boleh kosong.\n"); return; }

    Dokter* hasil = NULL;
    Dokter* d = head;

    while (d) {
        if (contains_keyword(d->tingkat, keyword)) {
            Dokter* nd = salin_dokter(d);
            if (nd) {
                nd->next = hasil;
                hasil = nd;
            }
        }
        d = d->next;
    }

    tampilkan_hasil(hasil);

    while (hasil) {
        Dokter* temp = hasil;
        hasil = hasil->next;
        free(temp);
    }
}

// Menu pencarian dengan submenu
void cari_dokter_menu() {
    int pilihan;
    do {
        printf("\n=== MENU CARI DOKTER ===\n");
        printf("1. Cari berdasarkan nama\n");
        printf("2. Cari berdasarkan bidang\n");
        printf("3. Cari berdasarkan tingkat\n");
        printf("0. Kembali ke menu utama\n");
        printf("Pilihan: ");
        if (scanf("%d", &pilihan) != 1) {
            printf("Input tidak valid. Harap masukkan angka.\n");
            while (getchar() != '\n');
            continue;
        }
        getchar(); // Konsumsi newline

        switch (pilihan) {
            case 1: cari_dokter_nama(); break;
            case 2: cari_dokter_bidang(); break;
            case 3: cari_dokter_tingkat(); break;
            case 0: printf("Kembali ke menu manajemen dokter...\n"); break;
            default: printf("Pilihan tidak valid.\n");
        }
    } while (pilihan != 0);
}

// --- SORTIR DOKTER ---
void sortir_dokter() {
    if (!head) {
        printf("Belum ada data dokter untuk disortir.\n");
        return;
    }

    int sortMode;
    printf("Sortir berdasarkan:\n");
    printf("1. Nama (A-Z)\n");
    printf("2. Max shift per minggu\n");
    printf("3. Tingkat\n");
    printf("4. Preferensi Shift\n");
    printf("5. Preferensi Waktu\n");
    printf("Pilihan: ");
    if (scanf("%d", &sortMode) != 1) {
        printf("Input tidak valid. Harap masukkan angka.\n");
        while (getchar() != '\n');
        return;
    }
    getchar(); // Konsumsi newline

    if (sortMode >= 1 && sortMode <= 5) {
        sort_dokter_list(&head, sortMode);
        printf("Dokter berhasil disortir dan disimpan.\n");
    } else {
        printf("Pilihan sortir tidak valid.\n");
    }
}

// --- STATISTIK ---
void statistik() {
    if (!head) {
        printf("Belum ada data dokter.\n");
        return;
    }

    int total = 0;
    long long sumShift = 0; // Gunakan long long untuk sumShift untuk mencegah overflow
    int maxShift = -1, minShift = 999999; // Inisialisasi minShift dengan nilai yang sangat besar
    Dokter *maxDokter = NULL, *minDokter = NULL;

    int countPagi = 0, countSiang = 0, countMalam = 0;
    int countCampur = 0, countAwalBulan = 0, countAkhirBulan = 0;

    // Untuk menghitung frekuensi bidang dan tingkat
    // Gunakan alokasi dinamis atau struktur data yang lebih efisien jika jumlah bidang/tingkat sangat banyak
    // Untuk tujuan ini, array statis dengan ukuran cukup besar sudah cukup.
    #define MAX_UNIQUE_CATEGORIES 50 // Misalnya, maksimum 50 bidang/tingkat unik

    typedef struct {
        char name[LEN];
        int count;
    } CategoryCount;

    CategoryCount bidangArr[MAX_UNIQUE_CATEGORIES];
    CategoryCount tingkatArr[MAX_UNIQUE_CATEGORIES];
    int bidangCount = 0;
    int tingkatCount = 0;

    Dokter* d = head;
    while (d) {
        total++;
        sumShift += d->max_shift_per_minggu;

        if (maxDokter == NULL || d->max_shift_per_minggu > maxShift) {
            maxShift = d->max_shift_per_minggu;
            maxDokter = d;
        }
        if (minDokter == NULL || d->max_shift_per_minggu < minShift) {
            minShift = d->max_shift_per_minggu;
            minDokter = d;
        }

        // Hitung preferensi shift
        if (strcmp(d->preferensi_shift, "Pagi") == 0) countPagi++;
        else if (strcmp(d->preferensi_shift, "Siang") == 0) countSiang++;
        else if (strcmp(d->preferensi_shift, "Malam") == 0) countMalam++;

        // Hitung preferensi waktu (case-insensitive)
        if (strcasecmp(d->preferensi_waktu, "Campur") == 0) countCampur++;
        else if (strcasecmp(d->preferensi_waktu, "Awal Bulan") == 0) countAwalBulan++;
        else if (strcasecmp(d->preferensi_waktu, "Akhir Bulan") == 0) countAkhirBulan++;

        // Hitung frekuensi bidang
        int foundBidang = 0;
        for (int i = 0; i < bidangCount; i++) {
            if (strcmp(bidangArr[i].name, d->bidang) == 0) {
                bidangArr[i].count++;
                foundBidang = 1;
                break;
            }
        }
        if (!foundBidang && bidangCount < MAX_UNIQUE_CATEGORIES) {
            strncpy(bidangArr[bidangCount].name, d->bidang, LEN - 1);
            bidangArr[bidangCount].name[LEN-1] = '\0';
            bidangArr[bidangCount].count = 1;
            bidangCount++;
        }

        // Hitung frekuensi tingkat
        int foundTingkat = 0;
        for (int i = 0; i < tingkatCount; i++) {
            if (strcmp(tingkatArr[i].name, d->tingkat) == 0) {
                tingkatArr[i].count++;
                foundTingkat = 1;
                break;
            }
        }
        if (!foundTingkat && tingkatCount < MAX_UNIQUE_CATEGORIES) {
            strncpy(tingkatArr[tingkatCount].name, d->tingkat, LEN - 1);
            tingkatArr[tingkatCount].name[LEN-1] = '\0';
            tingkatArr[tingkatCount].count = 1;
            tingkatCount++;
        }

        d = d->next;
    }

    printf("\n=== STATISTIK DATA DOKTER ===\n");
    printf("Total dokter: %d\n", total);
    if (total > 0) {
        printf("Rata-rata max shift/minggu: %.2f\n", (float)sumShift / total);
        if (maxDokter) printf("Dokter dengan max shift tertinggi: %s (%d shift)\n", maxDokter->nama, maxShift);
        if (minDokter) printf("Dokter dengan max shift terendah: %s (%d shift)\n", minDokter->nama, minShift);
    } else {
        printf("Tidak ada dokter untuk dihitung statistiknya.\n");
        return;
    }

    printf("\nJumlah dokter berdasarkan preferensi shift:\n");
    printf("- Pagi: %d\n", countPagi);
    printf("- Siang: %d\n", countSiang);
    printf("- Malam: %d\n", countMalam);

    printf("\nJumlah dokter berdasarkan preferensi waktu:\n");
    printf("- Campur: %d\n", countCampur);
    printf("- Awal Bulan: %d\n", countAwalBulan);
    printf("- Akhir Bulan: %d\n", countAkhirBulan);

    printf("\nJumlah dokter per bidang:\n");
    for (int i = 0; i < bidangCount; i++)
        printf("- %s: %d\n", bidangArr[i].name, bidangArr[i].count);

    printf("\nJumlah dokter per tingkat:\n");
    for (int i = 0; i < tingkatCount; i++)
        printf("- %s: %d\n", tingkatArr[i].name, tingkatArr[i].count);
}

// --- TAMPILKAN SEMUA DATA ---
void tampilkan_semua() {
    if (!head) {
        printf("Belum ada data dokter.\n");
        return;
    }
    Dokter* d = head;
    printf("\n=== DATA DOKTER ===\n");
    int i = 1;
    while (d) {
        printf("%d. Nama: %s\n", i++, d->nama);
        printf("   Bidang: %s\n", d->bidang);
        printf("   Tingkat: %s\n", d->tingkat);
        printf("   Max shift/minggu: %d\n", d->max_shift_per_minggu);
        printf("   Preferensi Shift: %s\n", d->preferensi_shift);
        printf("   Preferensi Waktu: %s\n", d->preferensi_waktu);
        printf("---------------------------------\n");
        d = d->next;
    }
}

// --- LOAD DATA DARI CSV ---
void load_data_dari_csv(const char *nama_file) {
    FILE *file = fopen(nama_file, "r");
    if (!file) {
        printf("Gagal membuka file %s: %s\n", nama_file, strerror(errno));
        char dir_path[] = "../data/";
        // Periksa apakah error adalah ENOENT (File or directory not found)
        if (errno == ENOENT) {
            // Coba buat direktori jika belum ada
#ifdef _WIN32
            // Untuk Windows
            _mkdir(dir_path);
#else
            // Untuk Unix/Linux/macOS
            mkdir(dir_path, 0755);
#endif
            printf("Direktori 'data/' mungkin sudah atau baru saja dibuat. Mencoba membuat file...\n");
            file = fopen(nama_file, "w+"); // Coba buka/buat file untuk read/write
            if (file) {
                fprintf(file, "nama,bidang,tingkat,max_shift_per_minggu,preferensi_shift,preferensi_waktu\n");
                printf("File %s berhasil dibuat dengan header.\n", nama_file);
                fclose(file);
                // Setelah membuat file, buka lagi dalam mode 'r' untuk membaca (meskipun kosong)
                file = fopen(nama_file, "r");
            } else {
                printf("Gagal membuat file %s setelah direktori dibuat: %s\n", nama_file, strerror(errno));
                return;
            }
        } else {
            // Error lain selain file/direktori tidak ditemukan
            return;
        }
    }

    if (!file) { // Periksa lagi jika file masih NULL setelah mencoba membuat
        printf("Gagal melanjutkan karena file %s tidak bisa dibuka.\n", nama_file);
        return;
    }

    char baris[256];
    int count = 0;

    // Baca header
    if (!fgets(baris, sizeof(baris), file)) {
        printf("File %s kosong atau hanya berisi header.\n", nama_file);
        fclose(file);
        return;
    }

    // Bebaskan linked list yang mungkin sudah ada sebelum memuat yang baru
    free_memory(); // Memastikan list kosong sebelum memuat yang baru

    while (fgets(baris, sizeof(baris), file)) {
        // Abaikan baris kosong
        if (strlen(baris) < 2) continue; // Minimal 1 karakter + newline
        
        Dokter* d = malloc(sizeof(Dokter));
        if (!d) {
            perror("Gagal alokasi memori saat memuat dokter");
            break; // Hentikan pembacaan jika alokasi gagal
        }
        d->next = NULL; // Inisialisasi next

        char *token;
        char temp_baris[256]; // Gunakan salinan baris untuk strtok
        strncpy(temp_baris, baris, sizeof(temp_baris) - 1);
        temp_baris[sizeof(temp_baris) - 1] = '\0';
        clean_newline(temp_baris); // Bersihkan newline dari salinan baris

        token = strtok(temp_baris, ",");
        if (!token) { free(d); continue; }
        strncpy(d->nama, token, LEN-1); d->nama[LEN-1] = '\0';

        token = strtok(NULL, ",");
        if (!token) { free(d); continue; }
        strncpy(d->bidang, token, LEN-1); d->bidang[LEN-1] = '\0';

        token = strtok(NULL, ",");
        if (!token) { free(d); continue; }
        strncpy(d->tingkat, token, LEN-1); d->tingkat[LEN-1] = '\0';

        token = strtok(NULL, ",");
        if (!token) { free(d); continue; }
        d->max_shift_per_minggu = atoi(token);

        token = strtok(NULL, ",");
        if (!token) { free(d); continue; }
        strncpy(d->preferensi_shift, token, LEN-1); d->preferensi_shift[LEN-1] = '\0';

        token = strtok(NULL, ","); // Token terakhir, bisa tanpa newline
        if (!token) { free(d); continue; }
        strncpy(d->preferensi_waktu, token, LEN-1); d->preferensi_waktu[LEN-1] = '\0';

        // Tambahkan ke linked list (ke head)
        d->next = head;
        head = d;
        count++;
    }

    fclose(file);
    printf("Berhasil memuat %d dokter dari %s\n", count, nama_file);
}

// --- SAVE DATA KE CSV ---
void save_data_to_csv(const char *nama_file) {
    FILE *file = fopen(nama_file, "w"); // Mode "w" akan membuat atau menimpa file
    if (!file) {
        printf("Gagal membuka file %s untuk menulis: %s\n", nama_file, strerror(errno));
        char dir_path[] = "../data/";
        if (errno == ENOENT) {
            // Coba buat direktori jika tidak ada
#ifdef _WIN32
            _mkdir(dir_path);
#else
            mkdir(dir_path, 0755);
#endif
            file = fopen(nama_file, "w"); // Coba buka lagi setelah membuat direktori
            if (!file) {
                printf("Gagal membuat file %s: %s\n", nama_file, strerror(errno));
                return;
            }
        } else {
            return;
        }
    }

    fprintf(file, "nama,bidang,tingkat,max_shift_per_minggu,preferensi_shift,preferensi_waktu\n");

    Dokter *d = head;
    while (d) {
        fprintf(file, "%s,%s,%s,%d,%s,%s\n",
                d->nama, d->bidang, d->tingkat,
                d->max_shift_per_minggu, d->preferensi_shift, d->preferensi_waktu);
        d = d->next;
    }
    fclose(file);
    printf("Data dokter berhasil disimpan ke %s\n", nama_file);
}

// --- FREE MEMORY ---
void free_memory() {
    while (head) {
        Dokter* temp = head;
        head = head->next;
        free(temp);
    }
    while (aktivitas_head) {
        Aktivitas* temp = aktivitas_head;
        aktivitas_head = aktivitas_head->next;
        free(temp->dokterData); // Bebaskan data dokter yang disalin
        free(temp); // Bebaskan node aktivitas
    }
}