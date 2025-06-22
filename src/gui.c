#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include header dari semua modul
#include "dokter.h"
#include "penjadwalan.h"
#include "laporan.h"
#include "laporan1.h"
#include "gui.h"

// Definisikan path file CSV
#define DOKTER_CSV_PATH "data/daftar_dokter.csv"
#define JADWAL_CSV_PATH "data/jadwal_dokter.csv"
#define LAPORAN_CSV_PATH "data/laporan_dokter.csv"

// Global variables untuk GTK widgets
static GtkWidget *main_window;
static GtkWidget *main_stack;
static GtkWidget *status_label;

// Forward declarations
static void show_main_menu(void);
static void show_doctor_management(void);
static void show_scheduling(void);
static void show_schedule_view(void);
static void show_performance_report(void);

// Utility function untuk menampilkan pesan status
static void set_status_message(const char *message) {
    if (status_label) {
        gtk_label_set_text(GTK_LABEL(status_label), message);
    }
}

// Utility function untuk menampilkan dialog pesan
static void show_message_dialog(GtkWindow *parent, const char *title, const char *message, GtkMessageType type) {
    GtkWidget *dialog = gtk_message_dialog_new(parent,
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               type,
                                               GTK_BUTTONS_OK,
                                               "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Callback functions untuk menu utama
static void on_doctor_management_clicked(GtkWidget *widget, gpointer data) {
    show_doctor_management();
}

static void on_scheduling_clicked(GtkWidget *widget, gpointer data) {
    show_scheduling();
}

static void on_schedule_view_clicked(GtkWidget *widget, gpointer data) {
    show_schedule_view();
}

static void on_performance_report_clicked(GtkWidget *widget, gpointer data) {
    show_performance_report();
}

static void on_exit_clicked(GtkWidget *widget, gpointer data) {
    // Simpan data sebelum keluar
    save_data_to_csv(DOKTER_CSV_PATH);
    free_memory();
    gtk_main_quit();
}

// Callback untuk kembali ke menu utama
static void on_back_to_main_clicked(GtkWidget *widget, gpointer data) {
    show_main_menu();
}

// Doctor Management callbacks
static void on_add_doctor_clicked(GtkWidget *widget, gpointer data) {
    tambah_dokter_manual();
    set_status_message("Dokter berhasil ditambahkan");
}

static void on_delete_doctor_clicked(GtkWidget *widget, gpointer data) {
    hapus_dokter();
    set_status_message("Dokter berhasil dihapus");
}

static void on_search_doctor_clicked(GtkWidget *widget, gpointer data) {
    cari_dokter_menu();
    set_status_message("Pencarian dokter selesai");
}

static void on_show_all_doctors_clicked(GtkWidget *widget, gpointer data) {
    tampilkan_semua();
    set_status_message("Menampilkan semua dokter");
}

static void on_undo_last_action_clicked(GtkWidget *widget, gpointer data) {
    hapus_aktivitas_terakhir();
    set_status_message("Aktivitas terakhir dibatalkan");
}

static void on_show_activity_log_clicked(GtkWidget *widget, gpointer data) {
    tampilkan_log();
    set_status_message("Menampilkan log aktivitas");
}

// Scheduling callback
static void on_generate_schedule_clicked(GtkWidget *widget, gpointer data) {
    set_status_message("Memuat data dokter...");
    
    if (!load_doctors_from_csv(DOKTER_CSV_PATH)) {
        show_message_dialog(GTK_WINDOW(main_window), "Error", 
                           "Gagal memuat data dokter atau file kosong", GTK_MESSAGE_ERROR);
        set_status_message("Gagal memuat data dokter");
        return;
    }
    
    set_status_message("Menghasilkan jadwal...");
    generate_schedule();
    
    set_status_message("Menyimpan jadwal...");
    save_schedule();
    
    show_message_dialog(GTK_WINDOW(main_window), "Sukses", 
                       "Penjadwalan berhasil dibuat dan disimpan", GTK_MESSAGE_INFO);
    set_status_message("Penjadwalan selesai");
}

// Schedule view callbacks
static void on_daily_schedule_clicked(GtkWidget *widget, gpointer data) {
    FILE *file_jadwal = fopen(JADWAL_CSV_PATH, "r");
    if (file_jadwal == NULL) {
        show_message_dialog(GTK_WINDOW(main_window), "Error", 
                           "Tidak dapat membuka file jadwal. Lakukan penjadwalan terlebih dahulu", GTK_MESSAGE_ERROR);
        return;
    }
    jalankanOpsi1(file_jadwal);
    fclose(file_jadwal);
    set_status_message("Menampilkan jadwal harian");
}

static void on_weekly_schedule_clicked(GtkWidget *widget, gpointer data) {
    FILE *file_jadwal = fopen(JADWAL_CSV_PATH, "r");
    if (file_jadwal == NULL) {
        show_message_dialog(GTK_WINDOW(main_window), "Error", 
                           "Tidak dapat membuka file jadwal. Lakukan penjadwalan terlebih dahulu", GTK_MESSAGE_ERROR);
        return;
    }
    jalankanOpsi2(file_jadwal);
    fclose(file_jadwal);
    set_status_message("Menampilkan jadwal mingguan");
}

static void on_monthly_schedule_clicked(GtkWidget *widget, gpointer data) {
    FILE *file_jadwal = fopen(JADWAL_CSV_PATH, "r");
    if (file_jadwal == NULL) {
        show_message_dialog(GTK_WINDOW(main_window), "Error", 
                           "Tidak dapat membuka file jadwal. Lakukan penjadwalan terlebih dahulu", GTK_MESSAGE_ERROR);
        return;
    }
    jalankanOpsi3(file_jadwal);
    fclose(file_jadwal);
    set_status_message("Menampilkan jadwal bulanan");
}

// Performance report callbacks
static void on_generate_report_clicked(GtkWidget *widget, gpointer data) {
    set_status_message("Memuat data untuk laporan...");
    
    load_shift_data(JADWAL_CSV_PATH);
    load_doctor_data(DOKTER_CSV_PATH);
    
    if (total_shift_entries == 0 || total_doctor_entries == 0) {
        show_message_dialog(GTK_WINDOW(main_window), "Error", 
                           "Tidak ada data yang cukup untuk membuat laporan", GTK_MESSAGE_ERROR);
        set_status_message("Gagal membuat laporan - data tidak cukup");
        return;
    }
    
    set_status_message("Memproses data...");
    proses_data();
    
    set_status_message("Menyimpan laporan...");
    tulis_laporan(LAPORAN_CSV_PATH);
    
    show_message_dialog(GTK_WINDOW(main_window), "Sukses", 
                       "Laporan kinerja dokter berhasil dibuat", GTK_MESSAGE_INFO);
    set_status_message("Laporan berhasil dibuat");
}

static void on_view_shift_totals_clicked(GtkWidget *widget, gpointer data) {
    lihat_jumlah_shift();
    set_status_message("Menampilkan total shift dokter");
}

static void on_view_violations_clicked(GtkWidget *widget, gpointer data) {
    lihat_pelanggaran();
    set_status_message("Menampilkan pelanggaran jadwal");
}

// Function untuk membuat button dengan styling
static GtkWidget* create_styled_button(const char *label, GCallback callback, gpointer data) {
    GtkWidget *button = gtk_button_new_with_label(label);
    gtk_widget_set_size_request(button, 250, 40);
    g_signal_connect(button, "clicked", callback, data);
    
    // Add CSS styling untuk macOS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "button { "
        "  background: linear-gradient(to bottom, #f0f0f0, #e0e0e0); "
        "  border: 1px solid #c0c0c0; "
        "  border-radius: 6px; "
        "  padding: 8px 16px; "
        "  font-weight: bold; "
        "} "
        "button:hover { "
        "  background: linear-gradient(to bottom, #e8e8e8, #d8d8d8); "
        "} "
        "button:active { "
        "  background: linear-gradient(to bottom, #d0d0d0, #c0c0c0); "
        "}",
        -1, NULL);
    
    GtkStyleContext *context = gtk_widget_get_style_context(button);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
    
    return button;
}

// Function untuk membuat main menu
static void show_main_menu(void) {
    // Clear existing content
    GList *children = gtk_container_get_children(GTK_CONTAINER(main_stack));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(main_box, 40);
    gtk_widget_set_margin_bottom(main_box, 40);
    gtk_widget_set_margin_start(main_box, 40);
    gtk_widget_set_margin_end(main_box, 40);
    
    // Title
    GtkWidget *title = gtk_label_new("Sistem Manajemen Jadwal Dokter");
    gtk_widget_set_name(title, "title-label");
    
    // Add CSS for title
    GtkCssProvider *title_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(title_provider,
        "#title-label { "
        "  font-size: 24px; "
        "  font-weight: bold; "
        "  color: #2c3e50; "
        "  margin-bottom: 20px; "
        "}",
        -1, NULL);
    
    GtkStyleContext *title_context = gtk_widget_get_style_context(title);
    gtk_style_context_add_provider(title_context, GTK_STYLE_PROVIDER(title_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(title_provider);
    
    gtk_box_pack_start(GTK_BOX(main_box), title, FALSE, FALSE, 0);
    
    // Menu buttons
    GtkWidget *btn_doctor = create_styled_button("1. Manajemen Dokter", G_CALLBACK(on_doctor_management_clicked), NULL);
    GtkWidget *btn_schedule = create_styled_button("2. Penjadwalan Otomatis", G_CALLBACK(on_scheduling_clicked), NULL);
    GtkWidget *btn_view = create_styled_button("3. Lihat Jadwal Dokter", G_CALLBACK(on_schedule_view_clicked), NULL);
    GtkWidget *btn_report = create_styled_button("4. Laporan Kinerja Dokter", G_CALLBACK(on_performance_report_clicked), NULL);
    GtkWidget *btn_exit = create_styled_button("0. Keluar Program", G_CALLBACK(on_exit_clicked), NULL);
    
    gtk_box_pack_start(GTK_BOX(main_box), btn_doctor, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), btn_schedule, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), btn_view, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), btn_report, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), btn_exit, FALSE, FALSE, 5);
    
    gtk_container_add(GTK_CONTAINER(main_stack), main_box);
    gtk_widget_show_all(main_stack);
    
    set_status_message("Selamat datang di Sistem Manajemen Jadwal Dokter");
}

// Function untuk menampilkan doctor management menu
static void show_doctor_management(void) {
    // Clear existing content
    GList *children = gtk_container_get_children(GTK_CONTAINER(main_stack));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_top(box, 30);
    gtk_widget_set_margin_bottom(box, 30);
    gtk_widget_set_margin_start(box, 30);
    gtk_widget_set_margin_end(box, 30);
    
    GtkWidget *title = gtk_label_new("MENU MANAJEMEN DOKTER");
    gtk_widget_set_name(title, "subtitle");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);
    
    GtkWidget *btn_add = create_styled_button("1. Tambah Dokter Baru", G_CALLBACK(on_add_doctor_clicked), NULL);
    GtkWidget *btn_delete = create_styled_button("2. Hapus Dokter", G_CALLBACK(on_delete_doctor_clicked), NULL);
    GtkWidget *btn_search = create_styled_button("3. Cari Dokter", G_CALLBACK(on_search_doctor_clicked), NULL);
    GtkWidget *btn_show_all = create_styled_button("4. Tampilkan Semua Dokter", G_CALLBACK(on_show_all_doctors_clicked), NULL);
    GtkWidget *btn_undo = create_styled_button("5. Batalkan Aktivitas Terakhir", G_CALLBACK(on_undo_last_action_clicked), NULL);
    GtkWidget *btn_log = create_styled_button("6. Tampilkan Log Aktivitas", G_CALLBACK(on_show_activity_log_clicked), NULL);
    GtkWidget *btn_back = create_styled_button("0. Kembali ke Menu Utama", G_CALLBACK(on_back_to_main_clicked), NULL);
    
    gtk_box_pack_start(GTK_BOX(box), btn_add, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(box), btn_delete, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(box), btn_search, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(box), btn_show_all, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(box), btn_undo, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(box), btn_log, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(box), btn_back, FALSE, FALSE, 3);
    
    gtk_container_add(GTK_CONTAINER(main_stack), box);
    gtk_widget_show_all(main_stack);
    
    set_status_message("Menu Manajemen Dokter");
}

// Function untuk menampilkan scheduling menu
static void show_scheduling(void) {
    // Clear existing content
    GList *children = gtk_container_get_children(GTK_CONTAINER(main_stack));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(box, 40);
    gtk_widget_set_margin_bottom(box, 40);
    gtk_widget_set_margin_start(box, 40);
    gtk_widget_set_margin_end(box, 40);
    
    GtkWidget *title = gtk_label_new("MENU PENJADWALAN OTOMATIS");
    gtk_widget_set_name(title, "subtitle");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);
    
    GtkWidget *info = gtk_label_new("Klik tombol di bawah untuk menghasilkan jadwal otomatis berdasarkan data dokter");
    gtk_label_set_line_wrap(GTK_LABEL(info), TRUE);
    gtk_box_pack_start(GTK_BOX(box), info, FALSE, FALSE, 10);
    
    GtkWidget *btn_generate = create_styled_button("Generate Jadwal Bulanan", G_CALLBACK(on_generate_schedule_clicked), NULL);
    GtkWidget *btn_back = create_styled_button("Kembali ke Menu Utama", G_CALLBACK(on_back_to_main_clicked), NULL);
    
    gtk_box_pack_start(GTK_BOX(box), btn_generate, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), btn_back, FALSE, FALSE, 5);
    
    gtk_container_add(GTK_CONTAINER(main_stack), box);
    gtk_widget_show_all(main_stack);
    
    set_status_message("Menu Penjadwalan Otomatis");
}

// Function untuk menampilkan schedule view menu
static void show_schedule_view(void) {
    // Clear existing content
    GList *children = gtk_container_get_children(GTK_CONTAINER(main_stack));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_top(box, 30);
    gtk_widget_set_margin_bottom(box, 30);
    gtk_widget_set_margin_start(box, 30);
    gtk_widget_set_margin_end(box, 30);
    
    GtkWidget *title = gtk_label_new("MENU LIHAT JADWAL");
    gtk_widget_set_name(title, "subtitle");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);
    
    GtkWidget *btn_daily = create_styled_button("1. Tampilkan Jadwal Harian", G_CALLBACK(on_daily_schedule_clicked), NULL);
    GtkWidget *btn_weekly = create_styled_button("2. Tampilkan Jadwal Mingguan", G_CALLBACK(on_weekly_schedule_clicked), NULL);
    GtkWidget *btn_monthly = create_styled_button("3. Tampilkan Jadwal Bulanan", G_CALLBACK(on_monthly_schedule_clicked), NULL);
    GtkWidget *btn_back = create_styled_button("0. Kembali ke Menu Utama", G_CALLBACK(on_back_to_main_clicked), NULL);
    
    gtk_box_pack_start(GTK_BOX(box), btn_daily, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), btn_weekly, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), btn_monthly, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), btn_back, FALSE, FALSE, 5);
    
    gtk_container_add(GTK_CONTAINER(main_stack), box);
    gtk_widget_show_all(main_stack);
    
    set_status_message("Menu Lihat Jadwal");
}

// Function untuk menampilkan performance report menu
static void show_performance_report(void) {
    // Clear existing content
    GList *children = gtk_container_get_children(GTK_CONTAINER(main_stack));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_top(box, 30);
    gtk_widget_set_margin_bottom(box, 30);
    gtk_widget_set_margin_start(box, 30);
    gtk_widget_set_margin_end(box, 30);
    
    GtkWidget *title = gtk_label_new("MENU LAPORAN KINERJA DOKTER");
    gtk_widget_set_name(title, "subtitle");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);
    
    GtkWidget *btn_generate = create_styled_button("Generate Laporan Kinerja", G_CALLBACK(on_generate_report_clicked), NULL);
    GtkWidget *btn_shifts = create_styled_button("1. Lihat Total Shift Dokter", G_CALLBACK(on_view_shift_totals_clicked), NULL);
    GtkWidget *btn_violations = create_styled_button("2. Lihat Pelanggaran Jadwal", G_CALLBACK(on_view_violations_clicked), NULL);
    GtkWidget *btn_back = create_styled_button("0. Kembali ke Menu Utama", G_CALLBACK(on_back_to_main_clicked), NULL);
    
    gtk_box_pack_start(GTK_BOX(box), btn_generate, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), btn_shifts, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), btn_violations, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), btn_back, FALSE, FALSE, 5);
    
    gtk_container_add(GTK_CONTAINER(main_stack), box);
    gtk_widget_show_all(main_stack);
    
    set_status_message("Menu Laporan Kinerja Dokter");
}

// Window close callback
static gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
    on_exit_clicked(NULL, NULL);
    return FALSE;
}

// Main GUI function
int gui_main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    
    // Load initial data
    load_data_dari_csv(DOKTER_CSV_PATH);
    
    // Create main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Sistem Manajemen Jadwal Dokter");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 600, 500);
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), FALSE);
    
    // Connect window close signal
    g_signal_connect(main_window, "delete-event", G_CALLBACK(on_window_delete_event), NULL);
    
    // Create main container
    GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), main_container);
    
    // Create stack for different views
    main_stack = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_container), main_stack, TRUE, TRUE, 0);
    
    // Create status bar
    status_label = gtk_label_new("Memuat aplikasi...");
    gtk_widget_set_margin_start(status_label, 10);
    gtk_widget_set_margin_end(status_label, 10);
    gtk_widget_set_margin_top(status_label, 5);
    gtk_widget_set_margin_bottom(status_label, 5);
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    
    // Add separator and status bar
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_container), separator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_container), status_label, FALSE, FALSE, 0);
    
    // Apply global CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { "
        "  background-color: #f5f5f5; "
        "} "
        "#subtitle { "
        "  font-size: 18px; "
        "  font-weight: bold; "
        "  color: #34495e; "
        "  margin-bottom: 15px; "
        "} ",
        -1, NULL);
    
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    
    // Show main menu
    show_main_menu();
    
    // Show window
    gtk_widget_show_all(main_window);
    
    // Start GTK main loop
    gtk_main();
    
    return 0;
}