#ifndef LAPORAN1_H
#define LAPORAN1_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define nama_file "jadwal_dokter.csv"


// =========================================================================
// DEFINISI VARIABEL GLOBAL
// =========================================================================
extern int tanggal_calendar;
extern char hari[100];
extern char shift[100];
extern char nama_dokter[100];
extern char nama_dokter2[1000][100];
extern char bidang[100];
extern char bidang2[1000][100];
extern char tingkat[100];
extern char tingkat2[1000][100];
extern char hari2[1000][100];
extern char shift2[1000][100];
extern char hari_next[100];
extern char shift_next[100];
extern char baris [100];
extern int jumlah_dokter_pershift;

// =========================================================================
// DEKLARASI FUNGSI 
// =========================================================================
void jalankanOpsi1(FILE* file);
void jalankanOpsi2(FILE* file);
void jalankanOpsi3(FILE* file);

#endif // LAPORAN1_H