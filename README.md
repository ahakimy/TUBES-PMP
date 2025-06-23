# Aplikasi Penjadwalan Dokter Otomatis Rumah Sakit

Ini adalah proyek Tugas Besar Pemecahan Masalah dengan Pemrograman (PMP) untuk membuat sistem penjadwalan dokter otomatis di RS Semoga Sehat Selalu.

# Deskripsi Proyek

Aplikasi ini bertujuan untuk mengotomatisasi proses penjadwalan jaga dokter di rumah sakit selama periode 30 hari. Setiap hari dibagi menjadi tiga *shift*: pagi, siang, dan malam. Setiap shift harus diisi oleh minimal satu dokter. Sistem ini mempertimbangkan batasan jumlah shift maksimal per minggu untuk setiap dokter serta preferensi shift (misalnya hanya ingin pagi, atau tidak ingin malam).

Tujuan utama adalah menghasilkan jadwal yang:
- Efisien dan adil.
- Menghindari beban kerja berlebih.
- Mengakomodasi preferensi pribadi dokter sebanyak mungkin.

# Fitur Utama

1. **Manajemen Data Dokter**
   - Tambah, hapus, dan tampilkan data dokter.
   - Data mencakup nama, bidang, tingkat, maksimal shift per minggu, preferensi shift, dan waktu (awal/akhir bulan).
   - Data disimpan dalam file `data/daftar_dokter.csv`.

2. **Penjadwalan Otomatis**
   - Jadwal dibuat untuk 30 hari ke depan, dengan total 90 shift (3 shift/hari).
   - Setiap shift diisi secara otomatis sesuai kebutuhan jumlah dokter, batasan beban kerja, dan preferensi yang ditentukan.

3. **Laporan dan Visualisasi Jadwal**
   - Laporan mencakup:
     - Jadwal harian.
     - Distribusi shift per dokter.
     - Jumlah pelanggaran terhadap preferensi.
   - Output dapat disimpan dalam file CSV (`data/jadwal_dokter.csv` dan `data/laporan_dokter.csv`).

4. **Antarmuka Pengguna (GUI)**
   - Menggunakan GTK+3 untuk tampilan grafis.
   - Navigasi antar menu utama, manajemen data dokter, penjadwalan, dan laporan.
   - Input dan output dilakukan melalui GUI sepenuhnya (tidak perlu terminal).
