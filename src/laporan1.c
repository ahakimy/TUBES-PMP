#include "laporan1.h"
#define nama_file "jadwal_dokter.csv"

int tanggal_calendar;
char hari[100];
char shift[100];
char nama_dokter[100];
char nama_dokter2[1000][100];
char bidang[100];
char bidang2[1000][100];
char tingkat[100];
char tingkat2[1000][100];
char hari2[1000][100];
char shift2[1000][100];
char hari_next[100];
char shift_next[100];
char baris[100];
int jumlah_dokter_pershift=2;

void jalankanOpsi1(FILE* file) {
    int i =0;
    int tanggal_user;
    printf("%-3s %-3s %-3s %-3s %-3s %-3s %-3s\n", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab", "Min");
    printf("---------------------------\n"); 
    int daysInMonth = 30;
    int firstDayOfMonthOffset = 0;
    for (int i = 0; i < firstDayOfMonthOffset; i++) {
        printf("    "); 
    }
    for (int day = 1; day <= daysInMonth; day++) {
        printf("%3d ", day);
        if ((day + firstDayOfMonthOffset) % 7 == 0) {
        printf("\n");
        }
    }
    if ((daysInMonth + firstDayOfMonthOffset) % 7 != 0) {
        printf("\n");
    }
    pengulangan_pilih_tanggal:
        printf("\nPilih tanggal (1-30): ");
        scanf("%d",&tanggal_user);getchar();
        if (tanggal_user>0 &&tanggal_user<30){
            rewind(file);
            while (fgets(baris,sizeof(baris),file)){
                sscanf(baris,"%d,%[^,],%[^,],%[^,],%[^,],%[^\n]",&tanggal_calendar,hari,shift,nama_dokter,bidang,tingkat);
                if (tanggal_calendar==tanggal_user){
                    strcpy(nama_dokter2[i],nama_dokter);
                    strcpy(tingkat2[i],tingkat);
                    strcpy(bidang2[i],bidang);
                    strcpy(hari2[i],hari);
                    strcpy(shift2[i],shift);
                    i++;                         
                }  
            }    
        
        printf ("------------------------------------------- Jadwal Tanggal ke - %d -------------------------------------------\n", tanggal_user);
        printf("\n%-5s | %-15s | %-20s | %-27s | %-14s | %-15s\n", "Tanggal", "Hari", "Nama Dokter", "Spesialisasi", "Tingkatan" , "Jam Praktek");
        printf("-------------------------------------------------------------------------------------------------------------\n");
        for (int i=0;i<(3*jumlah_dokter_pershift);i++){
                if (strcmp(hari_next,hari2[i])==0 && strcmp(shift_next,shift2[i])==0){
                printf("%-7s | %-15s | %-20s | %-27s | %-15s|%-15s\n" ," "," ", nama_dokter2[i], bidang2[i], tingkat2[i], "___________");
            }
            else if (strcmp(hari_next,hari2[i])==0){
                printf("%-7s | %-15s | %-20s | %-27s | %-15s| %-15s\n" ," "," ", nama_dokter2[i], bidang2[i], tingkat2[i], shift2[i]);
            }
        
            else {
                printf("-------------------------------------------------------------------------------------------------------------\n");                                                                                                                    
                printf("%-7d | %-15s | %-20s | %-27s | %-15s| %-15s\n" ,tanggal_user, hari2[i], nama_dokter2[i], bidang2[i], tingkat2[i], shift2[i]); 
            }
            strcpy(hari_next,hari2[i]);
            strcpy(shift_next,shift2[i]);

        }
    }
        else if (tanggal_user==30){
            rewind(file);
            while (fgets(baris,sizeof(baris),file)){
                sscanf(baris,"%d,%[^,],%[^,],%[^,],%[^,],%[^\n]",&tanggal_calendar,hari,shift,nama_dokter,bidang,tingkat);
                if (tanggal_calendar==tanggal_user){
                    strcpy(nama_dokter2[i],nama_dokter);
                    strcpy(tingkat2[i],tingkat);
                    strcpy(bidang2[i],bidang);
                    strcpy(hari2[i],hari);
                    strcpy(shift2[i],shift);
                    i++;                         
                }  
            }    
            printf ("------------------------------------------- Jadwal Tanggal ke - %d -------------------------------------------\n", tanggal_user);
            printf("\n%-5s | %-15s | %-20s | %-27s | %-14s | %-15s\n", "Tanggal", "Hari", "Nama Dokter", "Spesialisasi", "Tingkatan" , "Jam Praktek");
            printf("-------------------------------------------------------------------------------------------------------------\n");
            for (int i=1;i<=(3*jumlah_dokter_pershift);i++){
                    if (strcmp(hari_next,hari2[i])==0 && strcmp(shift_next,shift2[i])==0){
                    printf("%-7s | %-15s | %-20s | %-27s | %-15s|%-15s\n" ," "," ", nama_dokter2[i], bidang2[i], tingkat2[i], "___________");
                }
                else if (strcmp(hari_next,hari2[i])==0){
                    printf("%-7s | %-15s | %-20s | %-27s | %-15s| %-15s\n" ," "," ", nama_dokter2[i], bidang2[i], tingkat2[i], shift2[i]);
                }
            
                else {
                    printf("-------------------------------------------------------------------------------------------------------------\n");                                                                                                                    
                    printf("%-7d | %-15s | %-20s | %-27s | %-15s| %-15s\n" ,tanggal_user, hari2[i], nama_dokter2[i], bidang2[i], tingkat2[i], shift2[i]); 
                }
                strcpy(hari_next,hari2[i]);
                strcpy(shift_next,shift2[i]);

            }
        }
        
    else {
        printf("Tanggal tidak Valid\n");
        goto pengulangan_pilih_tanggal;
    }
    hari_next[0] = '\0';
    shift_next[0] = '\0';

}

// =========================================================================
// IMPLEMENTASI FUNGSI UNTUK OPSI 2 
// =========================================================================
void jalankanOpsi2(FILE* file) {
    int i=0;
    int minggu_user;
    printf("%-3s %-3s %-3s %-3s %-3s %-3s %-3s\n", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab", "Min");
    printf("---------------------------\n"); 
    int daysInMonth = 30;
    int firstDayOfMonthOffset = 0;
    for (int i = 0; i < firstDayOfMonthOffset; i++) {
        printf("    "); 
    }
    for (int day = 1; day <= daysInMonth; day++) {
        printf("%3d ", day);
        if ((day + firstDayOfMonthOffset) % 7 == 0) {
            printf("\n");
        }
    }
    if ((daysInMonth + firstDayOfMonthOffset) % 7 != 0) {
        printf("\n");
    }
    printf("\n");
    pengulangan_pilih_minggu:
    printf ("Pilih Minggu (1/2/3/4/5): ");
    scanf ("%d",&minggu_user);getchar();
    int m = (7*minggu_user)-6; 
    if (minggu_user>0&&minggu_user<6){
        rewind(file);
        while (fgets(baris,sizeof(baris),file)){
            sscanf(baris,"%d,%[^,],%[^,],%[^,],%[^,],%[^\n]",&tanggal_calendar,hari,shift,nama_dokter,bidang,tingkat);
            if (tanggal_calendar>= m &&tanggal_calendar<m+7){
                strcpy(nama_dokter2[i],nama_dokter);
                strcpy(tingkat2[i],tingkat);
                strcpy(bidang2[i],bidang);
                strcpy(hari2[i],hari);
                strcpy(shift2[i],shift);
                i++;                         
            }
        }
        printf ("-------------------------------------------- Jadwal Minggu ke - %d -------------------------------------------\n", minggu_user);
        printf("\n%-5s | %-15s | %-20s | %-27s | %-14s | %-15s\n", "Tanggal", "Hari", "Nama Dokter", "Spesialisasi", "Tingkatan" , "Jam Praktek");
        printf("-------------------------------------------------------------------------------------------------------------\n");     
        if (minggu_user==5){ 
            for (int i=1;i<=2*3*jumlah_dokter_pershift;i++){
                if (strcmp(hari_next,hari2[i])==0 && strcmp(shift_next,shift2[i])==0){
                    printf("%-7s | %-15s | %-20s | %-27s | %-15s|%-15s\n" ," "," ", nama_dokter2[i], bidang2[i], tingkat2[i], "___________");
                }
                else if (strcmp(hari_next,hari2[i])==0){
                    printf("%-7s | %-15s | %-20s | %-27s | %-15s| %-15s\n" ," "," ", nama_dokter2[i], bidang2[i], tingkat2[i], shift2[i]);
                }
            
                else {
                    printf("-------------------------------------------------------------------------------------------------------------\n");                                                                                                                    
                    printf("%-7d | %-15s | %-20s | %-27s | %-15s| %-15s\n" ,m, hari2[i], nama_dokter2[i], bidang2[i], tingkat2[i], shift2[i]); 
                    m+=1;
                }
                strcpy(hari_next,hari2[i]);
                strcpy(shift_next,shift2[i]);
            }
        }
        else {                                                                                                    
            for (int i=0;i<7*3*jumlah_dokter_pershift;i++){
                if (strcmp(hari_next,hari2[i])==0 && strcmp(shift_next,shift2[i])==0){
                    printf("%-7s | %-15s | %-20s | %-27s | %-15s|%-15s\n" ," "," ", nama_dokter2[i], bidang2[i], tingkat2[i], "___________");
                }
                else if (strcmp(hari_next,hari2[i])==0){
                    printf("%-7s | %-15s | %-20s | %-27s | %-15s| %-15s\n" ," "," ", nama_dokter2[i], bidang2[i], tingkat2[i], shift2[i]);
                }
            
                else {
                    printf("-------------------------------------------------------------------------------------------------------------\n");                                                                                                                    
                    printf("%-7d | %-15s | %-20s | %-27s | %-15s| %-15s\n" ,m, hari2[i], nama_dokter2[i], bidang2[i], tingkat2[i], shift2[i]); 
                    m+=1;
                }
                strcpy(hari_next,hari2[i]);
                strcpy(shift_next,shift2[i]);
            }
        }

    }
    else {
        printf("Input tidak Valid\n");
        goto pengulangan_pilih_minggu;
    }
    hari_next[0] = '\0';
    shift_next[0] = '\0';
}

// =========================================================================
// IMPLEMENTASI FUNGSI UNTUK OPSI 3
// =========================================================================
void jalankanOpsi3(FILE* file) {
    int i=0;
    int k=1;
    rewind(file);
    while (fgets(baris,sizeof(baris),file)){
        sscanf(baris,"%d,%[^,],%[^,],%[^,],%[^,],%[^\n]",&tanggal_calendar,hari,shift,nama_dokter,bidang,tingkat);
        strcpy(nama_dokter2[i],nama_dokter);
        strcpy(tingkat2[i],tingkat);
        strcpy(bidang2[i],bidang);
        strcpy(hari2[i],hari);
        strcpy(shift2[i],shift);
        i++;                         
            
    }
    printf ("----------------------------------------------- Jadwal Bulanan -----------------------------------------------\n");
    printf("\n%-5s | %-15s | %-20s | %-27s | %-14s | %-15s\n", "Tanggal", "Hari", "Nama Dokter", "Spesialisasi", "Tingkatan" , "Jam Praktek");
    printf("-------------------------------------------------------------------------------------------------------------\n");                                                                                                                    
    for (int i=1;i<=30*3*jumlah_dokter_pershift;i++){
        if (strcmp(hari_next,hari2[i])==0 && strcmp(shift_next,shift2[i])==0){
            printf("%-7s | %-15s | %-20s | %-27s | %-15s|%-15s\n" ," "," ", nama_dokter2[i], bidang2[i], tingkat2[i], "___________");
        }
        else if (strcmp(hari_next,hari2[i])==0){
            printf("%-7s | %-15s | %-20s | %-27s | %-15s| %-15s\n" ," "," ", nama_dokter2[i], bidang2[i], tingkat2[i], shift2[i]);
        }
        
        else {
            printf("-------------------------------------------------------------------------------------------------------------\n");                                                                                                                    
            printf("%-7d | %-15s | %-20s | %-27s | %-15s| %-15s\n" ,k, hari2[i], nama_dokter2[i], bidang2[i], tingkat2[i], shift2[i]); 
            k+=1;
        }
        strcpy(hari_next,hari2[i]);
        strcpy(shift_next,shift2[i]);
    }
    hari_next[0] = '\0';
    shift_next[0] = '\0';
}