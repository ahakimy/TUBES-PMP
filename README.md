# Aplikasi Penjadwalan Dokter Otomatis Rumah Sakit

Ini adalah proyek Tugas Besar Pemecahan Masalah dengan Pemrograman (PMP) untuk membuat sistem penjadwalan dokter otomatis di RS Semoga Sehat Selalu.

## Deskripsi Proyek

Aplikasi ini bertujuan untuk mengotomatisasi proses penjadwalan jaga dokter di Rumah sakit selama periode 30 hari[cite: 1, 19]. Setiap hari dibagi menjadi tiga *shift*: pagi, siang, dan malam. Setiap *shift* harus diisi oleh minimal satu dokter[cite: 2, 20]. Sistem ini akan mempertimbangkan batasan seperti jumlah *shift* maksimal per minggu untuk setiap dokter, serta preferensi *shift* masing-masing dokter (misalnya, tidak ingin *shift* malam)[cite: 3, 21].

Tujuan utama adalah untuk menghasilkan jadwal jaga yang efisien, memenuhi kebutuhan jumlah dokter di setiap *shift*, tidak melebihi beban kerja dokter, dan sebisa mungkin menghormati preferensi dokter[cite: 4, 22].

## Fitur Utama

Aplikasi ini akan memiliki fitur-fitur berikut:

1.  **Pengelolaan Data Dokter**:
    * Menambah, menghapus, dan menampilkan data dokter (nama, maksimal *shift* per minggu, dan preferensi *shift* seperti pagi/siang/malam).
    * Membaca data dokter dari file `daftar_dokter.csv`[cite: 7, 25].

2.  **Penentuan Jadwal Secara Otomatis**:
    * Menyusun jadwal jaga otomatis untuk 30 hari (total 90 *shift*)[cite: 8, 26].
    * Mengalokasikan *shift* dengan memperhatikan kebutuhan jumlah dokter, beban kerja, dan preferensi *shift* dokter[cite: 9, 27].

3.  **Menampilkan Informasi Jadwal**:
    * Memberikan informasi jadwal jaga harian, mingguan, dan bulanan.
    * Memberikan informasi jumlah *shift* masing-masing dokter.
    * Memberikan informasi jumlah pelanggaran preferensi *shift* dokter (jika ada).
    * Menyimpan jadwal yang dihasilkan ke file `jadwal.csv`.

## Struktur Proyek

Proyek ini diimplementasikan dalam Bahasa C dengan struktur modular (menggunakan beberapa file `.c` dan `.h`)[cite: 16].