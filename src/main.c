#include <stdio.h>
#include "penjadwalan.h"

int main() {
    if (!load_doctors_from_csv(INPUT_FILE)) {
        printf("Gagal membaca daftar dokter.\n");
        return 1;
    }

    generate_schedule();
    save_schedule();

    return 0;
}
