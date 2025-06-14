#include "dokter.h"

Dokter* head = NULL;
Aktivitas* aktivitas_head = NULL;

// Fungsi bantu membuat salinan dokter
Dokter* salin_dokter(Dokter* src) {
    if (!src) return NULL;
    Dokter* d = malloc(sizeof(Dokter));
    if (!d) return NULL;
    *d = *src;
    d->next = NULL;
    return d;
}

// --- MANAGE AKTIVITAS (UNDO) ---
void tambah_aktivitas(AksiType tipe, Dokter* data) {
    Aktivitas* a = malloc(sizeof(Aktivitas));
    if (!a) return;
    a->tipe = tipe;
    a->dokterData = salin_dokter(data); // Pastikan salinan dibuat
    if (!a->dokterData) {
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

    // Undo sesuai tipe aktivitas
    if (a->tipe == AKSI_TAMBAH) {
        Dokter *curr = head, *prev = NULL;
        while (curr) {
            if (strcmp(curr->nama, a->dokterData->nama) == 0) {
                if (prev) prev->next = curr->next;
                else head = curr->next;
                free(curr);
                printf("Undo: Penambahan dokter %s dibatalkan.\n", a->dokterData->nama);
                break;
            }
            prev = curr;
            curr = curr->next;
        }
    } else if (a->tipe == AKSI_HAPUS) {
        Dokter* d = salin_dokter(a->dokterData);
        if (d) {
            d->next = head;
            head = d;
            printf("Undo: Penghapusan dokter %s dibatalkan.\n", a->dokterData->nama);
        }
    }

    // Hanya bebaskan salinan, bukan data asli di linked list
    free(a->dokterData);
    free(a);
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
        printf("%d. %s dokter: %s\n", i++,
               curr->tipe == AKSI_TAMBAH ? "Tambah" : "Hapus",
               curr->dokterData->nama);
        curr = curr->next;
    }
}

// --- TAMBAH DOKTER ---
void tambah_dokter_manual() {
    Dokter* d = malloc(sizeof(Dokter));
    if (!d) return;

    printf("Nama: "); 
    if (!fgets(d->nama, LEN, stdin) || strlen(d->nama) >= LEN) { free(d); return; }
    d->nama[strcspn(d->nama, "\n")] = 0;

    printf("Bidang: "); 
    if (!fgets(d->bidang, LEN, stdin) || strlen(d->bidang) >= LEN) { free(d); return; }
    d->bidang[strcspn(d->bidang, "\n")] = 0;

    printf("Tingkat: "); 
    if (!fgets(d->tingkat, LEN, stdin) || strlen(d->tingkat) >= LEN) { free(d); return; }
    d->tingkat[strcspn(d->tingkat, "\n")] = 0;

    printf("Max shift/minggu: "); scanf("%d", &d->max_shift_per_minggu); getchar();
    printf("Preferensi Shift (Pagi/Siang/Malam): "); 
    if (!fgets(d->preferensi_shift, LEN, stdin) || strlen(d->preferensi_shift) >= LEN) { free(d); return; }
    d->preferensi_shift[strcspn(d->preferensi_shift, "\n")] = 0;

    printf("Preferensi Waktu (Campur/Awal Bulan/Akhir Bulan): "); 
    if (!fgets(d->preferensi_waktu, LEN, stdin) || strlen(d->preferensi_waktu) >= LEN) { free(d); return; }
    d->preferensi_waktu[strcspn(d->preferensi_waktu, "\n")] = 0;

    d->next = head;
    head = d;

    Dokter* copy = salin_dokter(d);
    tambah_aktivitas(AKSI_TAMBAH, copy);

    printf("Dokter %s berhasil ditambahkan.\n", d->nama);
}

// --- HAPUS DOKTER ---
void hapus_dokter() {
    char target[LEN];
    printf("Masukkan nama dokter yang ingin dihapus: ");
    if (!fgets(target, LEN, stdin) || strlen(target) >= LEN) return;
    target[strcspn(target, "\n")] = 0;

    Dokter *curr = head, *prev = NULL;
    while (curr) {
        if (strcmp(curr->nama, target) == 0) {
            char konfirmasi;
            printf("Apakah Anda yakin ingin menghapus dokter '%s'? (y/n): ", target);
            scanf(" %c", &konfirmasi); getchar();
            if (tolower(konfirmasi) != 'y') {
                printf("Penghapusan dibatalkan.\n");
                return;
            }

            if (prev) prev->next = curr->next;
            else head = curr->next;

            Dokter* copy = salin_dokter(curr);
            tambah_aktivitas(AKSI_HAPUS, copy);

            free(curr);
            printf("Dokter berhasil dihapus.\n");
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    printf("Dokter tidak ditemukan.\n");
}

// --- SORTING HELPER ---
int compare(const Dokter* a, const Dokter* b, int mode) {
    switch (mode) {
        case 1: return strcmp(a->nama, b->nama);
        case 2: return a->max_shift_per_minggu - b->max_shift_per_minggu;
        case 3: return strcmp(a->tingkat, b->tingkat);
        case 4: return strcmp(a->preferensi_shift, b->preferensi_shift);
        case 5: return strcmp(a->preferensi_waktu, b->preferensi_waktu);
        default: return 0;
    }
}

// Bubble sort linked list based on mode (perbaikan logika)
void sort_dokter_list(Dokter** headRef, int mode) {
    if (!(*headRef) || !(*headRef)->next) return;
    int swapped;
    Dokter *ptr1, *ptr2;
    do {
        swapped = 0;
        ptr1 = NULL;
        ptr2 = *headRef;
        while (ptr2->next) {
            if (compare(ptr2, ptr2->next, mode) > 0) {
                Dokter* temp = ptr2->next;
                ptr2->next = temp->next;
                temp->next = ptr2;
                if (ptr1) ptr1->next = temp;
                else *headRef = temp;
                ptr1 = temp;
                swapped = 1;
            } else {
                ptr1 = ptr2;
                ptr2 = ptr2->next;
            }
        }
    } while (swapped);
}

// Fungsi cek apakah string src mengandung keyword (case insensitive)
int contains_keyword(const char* src, const char* keyword) {
    if (!src || !keyword) return 0;
    size_t srcLen = strlen(src);
    size_t keyLen = strlen(keyword);
    if (srcLen >= LEN || keyLen >= LEN) return 0;

    char srcLow[LEN] = {0};
    char keyLow[LEN] = {0};
    for (size_t i = 0; i < srcLen && i < LEN-1; i++)
        srcLow[i] = tolower(src[i]);
    for (size_t i = 0; i < keyLen && i < LEN-1; i++)
        keyLow[i] = tolower(keyword[i]);

    return strstr(srcLow, keyLow) != NULL;
}

// Tampilkan hasil pencarian
void tampilkan_hasil(Dokter* hasil) {
    if (!hasil) {
        printf("Tidak ada data dokter.\n");
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
    if (!fgets(keyword, LEN, stdin) || strlen(keyword) >= LEN) return;
    keyword[strcspn(keyword, "\n")] = 0;

    Dokter* hasil = NULL;
    Dokter* d = head;

    while (d) {
        if (contains_keyword(d->nama, keyword)) {
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

// Cari dokter berdasarkan bidang
void cari_dokter_bidang() {
    char keyword[LEN];
    printf("Masukkan bidang dokter yang ingin dicari: ");
    if (!fgets(keyword, LEN, stdin) || strlen(keyword) >= LEN) return;
    keyword[strcspn(keyword, "\n")] = 0;

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
    if (!fgets(keyword, LEN, stdin) || strlen(keyword) >= LEN) return;
    keyword[strcspn(keyword, "\n")] = 0;

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
        scanf("%d", &pilihan); getchar();

        switch (pilihan) {
            case 1: cari_dokter_nama(); break;
            case 2: cari_dokter_bidang(); break;
            case 3: cari_dokter_tingkat(); break;
            case 0: printf("Kembali ke menu utama...\n"); break;
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
    scanf("%d", &sortMode); getchar();

    sort_dokter_list(&head, sortMode);
    printf("Dokter berhasil disortir.\n");
}

// --- STATISTIK ---
void statistik() {
    if (!head) {
        printf("Belum ada data dokter.\n");
        return;
    }

    int total = 0, sumShift = 0;
    int maxShift = -1, minShift = 1000000;
    Dokter *maxDokter = NULL, *minDokter = NULL;

    int countPagi = 0, countSiang = 0, countMalam = 0;
    int countCampur = 0, countAwalBulan = 0, countAkhirBulan = 0;

    typedef struct {
        char bidang[LEN];
        int count;
    } CountBidang;

    typedef struct {
        char tingkat[LEN];
        int count;
    } CountTingkat;

    CountBidang bidangArr[100];
    CountTingkat tingkatArr[100];
    int bidangCount = 0, tingkatCount = 0;

    Dokter* d = head;
    while (d) {
        total++;
        sumShift += d->max_shift_per_minggu;

        if (d->max_shift_per_minggu > maxShift) {
            maxShift = d->max_shift_per_minggu;
            maxDokter = d;
        }
        if (d->max_shift_per_minggu < minShift) {
            minShift = d->max_shift_per_minggu;
            minDokter = d;
        }

        if (strcmp(d->preferensi_shift, "Pagi") == 0) countPagi++;
        else if (strcmp(d->preferensi_shift, "Siang") == 0) countSiang++;
        else if (strcmp(d->preferensi_shift, "Malam") == 0) countMalam++;

        if (strcasecmp(d->preferensi_waktu, "Campur") == 0) countCampur++;
        else if (strcasecmp(d->preferensi_waktu, "Awal Bulan") == 0) countAwalBulan++;
        else if (strcasecmp(d->preferensi_waktu, "Akhir Bulan") == 0) countAkhirBulan++;

        int foundBidang = 0;
        for (int i = 0; i < bidangCount; i++) {
            if (strcmp(bidangArr[i].bidang, d->bidang) == 0) {
                bidangArr[i].count++;
                foundBidang = 1;
                break;
            }
        }
        if (!foundBidang) {
            strcpy(bidangArr[bidangCount].bidang, d->bidang);
            bidangArr[bidangCount].count = 1;
            bidangCount++;
        }

        int foundTingkat = 0;
        for (int i = 0; i < tingkatCount; i++) {
            if (strcmp(tingkatArr[i].tingkat, d->tingkat) == 0) {
                tingkatArr[i].count++;
                foundTingkat = 1;
                break;
            }
        }
        if (!foundTingkat) {
            strcpy(tingkatArr[tingkatCount].tingkat, d->tingkat);
            tingkatArr[tingkatCount].count = 1;
            tingkatCount++;
        }

        d = d->next;
    }

    printf("\n=== STATISTIK DATA DOKTER ===\n");
    printf("Total dokter: %d\n", total);
    printf("Rata-rata max shift/minggu: %.2f\n", (float)sumShift / total);
    printf("Dokter dengan max shift tertinggi: %s (%d shift)\n", maxDokter->nama, maxShift);
    printf("Dokter dengan max shift terendah: %s (%d shift)\n", minDokter->nama, minShift);

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
        printf("- %s: %d\n", bidangArr[i].bidang, bidangArr[i].count);

    printf("\nJumlah dokter per tingkat:\n");
    for (int i = 0; i < tingkatCount; i++)
        printf("- %s: %d\n", tingkatArr[i].tingkat, tingkatArr[i].count);
}

// --- TAMPILKAN SEMUA DATA ---
void tampilkan_semua() {
    if (!head) {
        printf("Belum ada data dokter.\n");
        return;
    }
    Dokter* d = head;
    printf("\n=== DATA DOKTER ===\n");
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

// --- LOAD DATA DARI CSV ---
void load_data_dari_csv(const char *nama_file) {
    FILE *file = fopen(nama_file, "r");
    if (!file) {
        printf("Gagal membuka file %s\n", nama_file);
        return;
    }

    char baris[256];
    int count = 0;

    // Skip header
    fgets(baris, sizeof(baris), file);

    while (fgets(baris, sizeof(baris), file) && count < MAX_DOKTER) {
        Dokter* d = malloc(sizeof(Dokter));
        if (!d) continue;

        char *token = strtok(baris, ",");
        if (!token) { free(d); continue; }
        strncpy(d->nama, token, LEN-1); d->nama[LEN-1] = 0;

        token = strtok(NULL, ",");
        if (!token) { free(d); continue; }
        strncpy(d->bidang, token, LEN-1); d->bidang[LEN-1] = 0;

        token = strtok(NULL, ",");
        if (!token) { free(d); continue; }
        strncpy(d->tingkat, token, LEN-1); d->tingkat[LEN-1] = 0;

        token = strtok(NULL, ",");
        if (!token) { free(d); continue; }
        d->max_shift_per_minggu = atoi(token);

        token = strtok(NULL, ",");
        if (!token) { free(d); continue; }
        strncpy(d->preferensi_shift, token, LEN-1); d->preferensi_shift[LEN-1] = 0;

        token = strtok(NULL, ",\n");
        if (!token) { free(d); continue; }
        char *newline = strchr(token, '\n');
        if (newline) *newline = 0;
        strncpy(d->preferensi_waktu, token, LEN-1); d->preferensi_waktu[LEN-1] = 0;

        d->next = head;
        head = d;
        count++;
    }

    fclose(file);
    printf("Berhasil memuat %d dokter dari %s\n", count, nama_file);
}

// --- SAVE DATA KE CSV ---
void save_data_to_csv(const char *nama_file) {
    FILE *file = fopen(OUTPUT_FILE, "w");
    if (!file) {
        printf("Gagal membuka file %s untuk menulis\n", OUTPUT_FILE);
        return;
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
    printf("Data dokter berhasil disimpan ke %s\n", OUTPUT_FILE);
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
        free(temp->dokterData);
        free(temp);
    }
}

// --- MAIN MENU ---
void menu() {
    // Load data awal saat program dimulai
    load_data_dari_csv(OUTPUT_FILE);

    int pilihan;
    do {
        printf("\n=== MENU ===\n");
        printf("1. Tambah dokter\n");
        printf("2. Hapus dokter\n");
        printf("3. Cari dokter\n");
        printf("4. Sortir dokter\n");
        printf("5. Tampilkan semua dokter\n");
        printf("6. Statistik\n");
        printf("7. Undo aktivitas terakhir\n");
        printf("8. Tampilkan log aktivitas\n");
        printf("0. Keluar\n");
        printf("Pilihan: ");
        scanf("%d", &pilihan); getchar();

        switch(pilihan) {
            case 1: tambah_dokter_manual(); break;
            case 2: hapus_dokter(); break;
            case 3: cari_dokter_menu(); break;
            case 4: sortir_dokter(); break;
            case 5: tampilkan_semua(); break;
            case 6: statistik(); break;
            case 7: hapus_aktivitas_terakhir(); break;
            case 8: tampilkan_log(); break;
            case 0: 
                printf("Menyimpan data sebelum keluar...\n");
                save_data_to_csv(OUTPUT_FILE);
                free_memory(); // Bebaskan memori sebelum keluar
                printf("Keluar program...\n"); 
                break;
            default: printf("Pilihan tidak valid.\n");
        }
    } while (pilihan != 0);
}