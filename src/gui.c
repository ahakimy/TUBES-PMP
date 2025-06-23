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
static void show_search_results(Dokter *hasil);
static void show_schedule_tree_view(FILE *file, int filter_value, int filter_value2, int mode);

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

// Function untuk membuat button dengan styling
static GtkWidget* create_styled_button(const char *label, GCallback callback, gpointer data) {
    GtkWidget *button = gtk_button_new_with_label(label);
    gtk_widget_set_size_request(button, 250, 40);
    g_signal_connect(button, "clicked", callback, data);
    
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

// Callback untuk dialog tambah dokter
static void on_add_doctor_dialog_response(GtkDialog *dialog, gint response_id, gpointer data) {
    if (response_id == GTK_RESPONSE_OK) {
        GtkWidget **entries = (GtkWidget **)data;
        const char *nama = gtk_entry_get_text(GTK_ENTRY(entries[0]));
        const char *bidang = gtk_entry_get_text(GTK_ENTRY(entries[1]));
        const char *tingkat = gtk_entry_get_text(GTK_ENTRY(entries[2]));
        const char *max_shift = gtk_entry_get_text(GTK_ENTRY(entries[3]));
        const char *pref_shift = gtk_entry_get_text(GTK_ENTRY(entries[4]));
        const char *pref_waktu = gtk_entry_get_text(GTK_ENTRY(entries[5]));

        if (strlen(nama) == 0 || strlen(bidang) == 0 || strlen(tingkat) == 0 ||
            strlen(max_shift) == 0 || strlen(pref_shift) == 0 || strlen(pref_waktu) == 0) {
            show_message_dialog(GTK_WINDOW(dialog), "Error", "Semua field harus diisi", GTK_MESSAGE_ERROR);
            return;
        }

        Dokter *d = malloc(sizeof(Dokter));
        if (!d) {
            show_message_dialog(GTK_WINDOW(dialog), "Error", "Gagal alokasi memori", GTK_MESSAGE_ERROR);
            gtk_widget_destroy(GTK_WIDGET(dialog));
            return;
        }

        strncpy(d->nama, nama, LEN-1); d->nama[LEN-1] = '\0';
        strncpy(d->bidang, bidang, LEN-1); d->bidang[LEN-1] = '\0';
        strncpy(d->tingkat, tingkat, LEN-1); d->tingkat[LEN-1] = '\0';
        d->max_shift_per_minggu = atoi(max_shift);
        strncpy(d->preferensi_shift, pref_shift, LEN-1); d->preferensi_shift[LEN-1] = '\0';
        strncpy(d->preferensi_waktu, pref_waktu, LEN-1); d->preferensi_waktu[LEN-1] = '\0';

        d->next = head;
        head = d;

        Dokter *copy_for_undo = salin_dokter(d);
        if (copy_for_undo) {
            tambah_aktivitas(AKSI_TAMBAH, copy_for_undo);
        } else {
            show_message_dialog(GTK_WINDOW(dialog), "Peringatan", "Gagal membuat salinan untuk undo", GTK_MESSAGE_WARNING);
        }

        save_data_to_csv(DOKTER_CSV_PATH);
        set_status_message("Dokter berhasil ditambahkan");
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_free(data);
}

// Function untuk menampilkan dialog tambah dokter
static void show_add_doctor_dialog(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data; // Suppress unused parameter warnings
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Tambah Dokter Baru",
                                                    GTK_WINDOW(main_window),
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 20);

    GtkWidget *labels[] = {
        gtk_label_new("Nama:"), gtk_label_new("Bidang:"), gtk_label_new("Tingkat:"),
        gtk_label_new("Max Shift/Minggu:"), gtk_label_new("Preferensi Shift:"),
        gtk_label_new("Preferensi Waktu:")
    };
    GtkWidget *entries[6];
    for (int i = 0; i < 6; i++) {
        entries[i] = gtk_entry_new();
        gtk_grid_attach(GTK_GRID(grid), labels[i], 0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), entries[i], 1, i, 1, 1);
    }

    gtk_container_add(GTK_CONTAINER(content_area), grid);

    GtkWidget **entry_data = g_new(GtkWidget *, 6);
    memcpy(entry_data, entries, sizeof(entries));
    g_signal_connect(dialog, "response", G_CALLBACK(on_add_doctor_dialog_response), entry_data);

    gtk_widget_show_all(dialog);
}

// Callback untuk dialog hapus dokter
static void on_delete_doctor_dialog_response(GtkDialog *dialog, gint response_id, gpointer data) {
    if (response_id == GTK_RESPONSE_OK) {
        const char *nama = gtk_entry_get_text(GTK_ENTRY(data));
        if (strlen(nama) == 0) {
            show_message_dialog(GTK_WINDOW(dialog), "Error", "Nama dokter harus diisi", GTK_MESSAGE_ERROR);
            return;
        }

        Dokter *curr = head, *prev = NULL;
        while (curr) {
            if (strcmp(curr->nama, nama) == 0) {
                if (prev) prev->next = curr->next;
                else head = curr->next;

                Dokter *copy_for_undo = salin_dokter(curr);
                if (copy_for_undo) {
                    tambah_aktivitas(AKSI_HAPUS, copy_for_undo);
                } else {
                    show_message_dialog(GTK_WINDOW(dialog), "Peringatan", "Gagal membuat salinan untuk undo", GTK_MESSAGE_WARNING);
                }

                free(curr);
                save_data_to_csv(DOKTER_CSV_PATH);
                set_status_message("Dokter berhasil dihapus");
                gtk_widget_destroy(GTK_WIDGET(dialog));
                g_free(data);
                return;
            }
            prev = curr;
            curr = curr->next;
        }
        show_message_dialog(GTK_WINDOW(dialog), "Error", "Dokter tidak ditemukan", GTK_MESSAGE_ERROR);
    } else {
        gtk_widget_destroy(GTK_WIDGET(dialog));
        g_free(data);
    }
}

// Function untuk menampilkan dialog hapus dokter
static void show_delete_doctor_dialog(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Hapus Dokter",
                                                    GTK_WINDOW(main_window),
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 150);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);

    GtkWidget *label = gtk_label_new("Masukkan nama dokter yang ingin dihapus:");
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    g_signal_connect(dialog, "response", G_CALLBACK(on_delete_doctor_dialog_response), entry);
    gtk_widget_show_all(dialog);
}

// Callback untuk dialog pencarian dokter
static void on_search_doctor_dialog_response(GtkDialog *dialog, gint response_id, gpointer data) {
    if (response_id == GTK_RESPONSE_OK) {
        GtkWidget **widgets = (GtkWidget **)data;
        const char *keyword = gtk_entry_get_text(GTK_ENTRY(widgets[0]));
        int search_type = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets[1]));

        if (strlen(keyword) == 0) {
            show_message_dialog(GTK_WINDOW(dialog), "Error", "Keyword harus diisi", GTK_MESSAGE_ERROR);
            return;
        }

        Dokter *hasil = NULL;
        Dokter *d = head;

        while (d) {
            int match = 0;
            switch (search_type) {
                case 0: // Nama
                    match = contains_keyword(d->nama, keyword);
                    break;
                case 1: // Bidang
                    match = contains_keyword(d->bidang, keyword);
                    break;
                case 2: // Tingkat
                    match = contains_keyword(d->tingkat, keyword);
                    break;
            }
            if (match) {
                Dokter *nd = salin_dokter(d);
                if (nd) {
                    nd->next = hasil;
                    hasil = nd;
                }
            }
            d = d->next;
        }

        show_search_results(hasil);

        while (hasil) {
            Dokter *temp = hasil;
            hasil = hasil->next;
            free(temp);
        }
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_free(data);
}

// Function untuk menampilkan dialog pencarian dokter
static void show_search_doctor_dialog(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Cari Dokter",
                                                    GTK_WINDOW(main_window),
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 200);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);

    GtkWidget *label = gtk_label_new("Masukkan keyword pencarian:");
    GtkWidget *entry = gtk_entry_new();
    GtkWidget *combo_label = gtk_label_new("Pilih kriteria pencarian:");
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "Nama");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "Bidang");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "Tingkat");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), combo_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    GtkWidget **widgets = g_new(GtkWidget *, 2);
    widgets[0] = entry;
    widgets[1] = combo;
    g_signal_connect(dialog, "response", G_CALLBACK(on_search_doctor_dialog_response), widgets);

    gtk_widget_show_all(dialog);
}

// Function untuk menampilkan hasil pencarian dalam tree view
static void show_search_results(Dokter *hasil) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Hasil Pencarian Dokter");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    if (!hasil) {
        GtkWidget *label = gtk_label_new("Tidak ada dokter yang cocok dengan kriteria pencarian.");
        gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
    } else {
        GtkListStore *store = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                                 G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
        GtkTreeIter iter;
        Dokter *d = hasil;
        while (d) {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                               0, d->nama,
                               1, d->bidang,
                               2, d->tingkat,
                               3, d->max_shift_per_minggu,
                               4, d->preferensi_shift,
                               5, d->preferensi_waktu,
                               -1);
            d = d->next;
        }

        GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        g_object_unref(store);

        const char *titles[] = {"Nama", "Bidang", "Tingkat", "Max Shift/Minggu", "Pref. Shift", "Pref. Waktu"};
        for (int i = 0; i < 6; i++) {
            GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
            GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
        }

        GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_container_add(GTK_CONTAINER(scrolled), tree);
        gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
    }

    GtkWidget *close_button = create_styled_button("Tutup", G_CALLBACK(gtk_widget_destroy), window);
    gtk_box_pack_start(GTK_BOX(vbox), close_button, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
    set_status_message("Pencarian dokter selesai");
}

// Function untuk menampilkan semua dokter dalam tree view
static void show_all_doctors(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_search_results(head);
    set_status_message("Menampilkan semua dokter");
}

// Function untuk menampilkan dialog undo
static void show_undo_dialog(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    if (!aktivitas_head) {
        show_message_dialog(GTK_WINDOW(main_window), "Info", "Tidak ada aktivitas untuk dibatalkan", GTK_MESSAGE_INFO);
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Batalkan Aktivitas Terakhir",
                                                    GTK_WINDOW(main_window),
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 150);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);

    char msg[256];
    snprintf(msg, sizeof(msg), "Batalkan %s dokter: %s?", 
             aktivitas_head->tipe == AKSI_TAMBAH ? "penambahan" : "penghapusan",
             aktivitas_head->dokterData ? aktivitas_head->dokterData->nama : "tidak diketahui");
    GtkWidget *label = gtk_label_new(msg);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    g_signal_connect(dialog, "response", G_CALLBACK(hapus_aktivitas_terakhir), NULL);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);

    gtk_widget_show_all(dialog);
    set_status_message("Menampilkan dialog undo");
}

// Function untuk menampilkan log aktivitas
static void show_activity_log(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Log Aktivitas");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    if (!aktivitas_head) {
        gtk_text_buffer_set_text(buffer, "Belum ada aktivitas.\n", -1);
    } else {
        GString *log = g_string_new("");
        Aktivitas *curr = aktivitas_head;
        int i = 1;
        while (curr) {
            if (curr->dokterData) {
                g_string_append_printf(log, "%d. %s dokter: %s\n", i++,
                                       curr->tipe == AKSI_TAMBAH ? "Tambah" : "Hapus",
                                       curr->dokterData->nama);
            } else {
                g_string_append_printf(log, "%d. Aktivitas tidak diketahui (data dokter hilang)\n", i++);
            }
            curr = curr->next;
        }
        gtk_text_buffer_set_text(buffer, log->str, -1);
        g_string_free(log, TRUE);
    }

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

    GtkWidget *close_button = create_styled_button("Tutup", G_CALLBACK(gtk_widget_destroy), window);
    gtk_box_pack_start(GTK_BOX(vbox), close_button, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
    set_status_message("Menampilkan log aktivitas");
}

// Function untuk menampilkan jadwal dalam tree view
static void show_schedule_tree_view(FILE *file, int filter_value, int filter_value2, int mode) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    char title[50];
    if (mode == 1) snprintf(title, sizeof(title), "Jadwal Harian - Tanggal %d", filter_value);
    else if (mode == 2) snprintf(title, sizeof(title), "Jadwal Mingguan - Minggu %d", filter_value);
    else snprintf(title, sizeof(title), "Jadwal Bulanan");
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    GtkListStore *store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING,
                                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkTreeIter iter;

    char line[1024];
    fgets(line, sizeof(line), file); // Skip header
    int start_day = mode == 2 ? (filter_value - 1) * 7 + 1 : 1;
    int end_day = mode == 2 ? start_day + 6 : 30;
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        int tanggal;
        char hari[100], shift[100], nama[100], bidang[100], tingkat[100];
        sscanf(line, "%d,%[^,],%[^,],%[^,],%[^,],%s", &tanggal, hari, shift, nama, bidang, tingkat);

        if ((mode == 1 && tanggal == filter_value) ||
            (mode == 2 && tanggal >= start_day && tanggal <= end_day) ||
            (mode == 3)) {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                               0, tanggal,
                               1, hari,
                               2, nama,
                               3, bidang,
                               4, tingkat,
                               5, shift,
                               -1);
            count++;
        }
    }

    if (count == 0) {
        GtkWidget *label = gtk_label_new("Tidak ada data jadwal untuk kriteria yang dipilih.");
        gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
    } else {
        GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        g_object_unref(store);

        const char *titles[] = {"Tanggal", "Hari", "Nama", "Spesialisasi", "Tingkat", "Shift"};
        for (int i = 0; i < 6; i++) {
            GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
            GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
        }

        GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_container_add(GTK_CONTAINER(scrolled), tree);
        gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
    }

    GtkWidget *close = create_styled_button("Tutup", G_CALLBACK(gtk_widget_destroy), window);
    gtk_box_pack_start(GTK_BOX(vbox), close, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
    set_status_message(mode == 1 ? "Menampilkan jadwal harian" : mode == 2 ? "Menampilkan jadwal mingguan" : "Menampilkan jadwal bulanan");
}

// Callback untuk dialog jadwal harian
static void on_daily_schedule_dialog_response(GtkDialog *dialog, gint response_id, gpointer data) {
    if (response_id == GTK_RESPONSE_OK) {
        const char *input = gtk_entry_get_text(GTK_ENTRY(data));
        int tanggal = atoi(input);
        if (tanggal < 1 || tanggal > 30) {
            show_message_dialog(GTK_WINDOW(dialog), "Error", "Tanggal tidak valid (harus 1-30)", GTK_MESSAGE_ERROR);
            return;
        }

        FILE *file = fopen(JADWAL_CSV_PATH, "r");
        if (!file) {
            show_message_dialog(GTK_WINDOW(dialog), "Error", "Tidak dapat membuka file jadwal", GTK_MESSAGE_ERROR);
            gtk_widget_destroy(GTK_WIDGET(dialog));
            return;
        }

        show_schedule_tree_view(file, tanggal, 0, 1); // Mode 1: Harian
        fclose(file);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

// Function untuk menampilkan dialog jadwal harian
static void show_daily_schedule_dialog(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Pilih Tanggal",
                                                    GTK_WINDOW(main_window),
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 150);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);

    GtkWidget *label = gtk_label_new("Masukkan tanggal (1-30):");
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    g_signal_connect(dialog, "response", G_CALLBACK(on_daily_schedule_dialog_response), entry);
    gtk_widget_show_all(dialog);
}

// Callback untuk dialog jadwal mingguan
static void on_weekly_schedule_dialog_response(GtkDialog *dialog, gint response_id, gpointer data) {
    if (response_id == GTK_RESPONSE_OK) {
        const char *input = gtk_entry_get_text(GTK_ENTRY(data));
        int minggu = atoi(input);
        if (minggu < 1 || minggu > 5) {
            show_message_dialog(GTK_WINDOW(dialog), "Error", "Minggu tidak valid (harus 1-5)", GTK_MESSAGE_ERROR);
            return;
        }

        FILE *file = fopen(JADWAL_CSV_PATH, "r");
        if (!file) {
            show_message_dialog(GTK_WINDOW(dialog), "Error", "Tidak dapat membuka file jadwal", GTK_MESSAGE_ERROR);
            gtk_widget_destroy(GTK_WIDGET(dialog));
            return;
        }

        show_schedule_tree_view(file, minggu, 0, 2); // Mode 2: Mingguan
        fclose(file);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

// Function untuk menampilkan dialog jadwal mingguan
static void show_weekly_schedule_dialog(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Pilih Minggu",
                                                    GTK_WINDOW(main_window),
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 150);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);

    GtkWidget *label = gtk_label_new("Masukkan minggu (1-5):");
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    g_signal_connect(dialog, "response", G_CALLBACK(on_weekly_schedule_dialog_response), entry);
    gtk_widget_show_all(dialog);
}

// Function untuk menampilkan jadwal bulanan
static void show_monthly_schedule(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    FILE *file = fopen(JADWAL_CSV_PATH, "r");
    if (!file) {
        show_message_dialog(GTK_WINDOW(main_window), "Error", "Tidak dapat membuka file jadwal", GTK_MESSAGE_ERROR);
        return;
    }

    show_schedule_tree_view(file, 0, 0, 3); // Mode 3: Bulanan
    fclose(file);
}

// Callback untuk dialog jumlah shift
static void on_shift_totals_dialog_response(GtkDialog *dialog, gint response_id, gpointer data) {
    if (response_id == GTK_RESPONSE_OK) {
        const char *nama_dokter = gtk_entry_get_text(GTK_ENTRY(data));
        if (strlen(nama_dokter) == 0) {
            show_message_dialog(GTK_WINDOW(dialog), "Error", "Nama dokter harus diisi", GTK_MESSAGE_ERROR);
            return;
        }

        FILE *fp = fopen(LAPORAN_CSV_PATH, "r");
        if (!fp) {
            show_message_dialog(GTK_WINDOW(dialog), "Error", "Gagal membuka file laporan", GTK_MESSAGE_ERROR);
            gtk_widget_destroy(GTK_WIDGET(dialog));
            return;
        }

        char line[1024];
        fgets(line, sizeof(line), fp); // Skip header
        int found = 0;
        char msg[256];

        while (fgets(line, sizeof(line), fp)) {
            char *token = strtok(line, ",");
            if (token && strcmp(token, nama_dokter) == 0) {
                found = 1;
                for (int i = 0; i < 2; i++) strtok(NULL, ",");
                char *total_shift = strtok(NULL, ",");
                snprintf(msg, sizeof(msg), "Total shift untuk dokter %s: %s", nama_dokter, total_shift);
                show_message_dialog(GTK_WINDOW(dialog), "Total Shift", msg, GTK_MESSAGE_INFO);
                break;
            }
        }

        if (!found) {
            snprintf(msg, sizeof(msg), "Dokter dengan nama '%s' tidak ditemukan.", nama_dokter);
            show_message_dialog(GTK_WINDOW(dialog), "Error", msg, GTK_MESSAGE_ERROR);
        }

        fclose(fp);
        set_status_message("Menampilkan total shift dokter");
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

// Function untuk menampilkan dialog jumlah shift
static void show_shift_totals_dialog(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Lihat Total Shift",
                                                    GTK_WINDOW(main_window),
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 150);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);

    GtkWidget *label = gtk_label_new("Masukkan nama dokter:");
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    g_signal_connect(dialog, "response", G_CALLBACK(on_shift_totals_dialog_response), entry);
    gtk_widget_show_all(dialog);
}

// Function untuk menampilkan pelanggaran dalam text view
static void show_violations_view(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    load_shift_data(JADWAL_CSV_PATH);
    load_doctor_data(DOKTER_CSV_PATH);

    if (total_shift_entries == 0 || total_doctor_entries == 0) {
        show_message_dialog(GTK_WINDOW(main_window), "Error", "Tidak ada data untuk menampilkan pelanggaran", GTK_MESSAGE_ERROR);
        return;
    }

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Pelanggaran Jadwal");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    GString *output = g_string_new("");

    int minggu_saat_ini = 0;
    int tanggal_ke_minggu[32] = {0};
    for (int i = 0; i < total_shift_entries; i++) {
        ShiftEntry *e = &shift_data[i];
        if (strcmp(e->hari, "Senin") == 0 && tanggal_ke_minggu[e->tanggal] == 0) {
            minggu_saat_ini++;
        }
        tanggal_ke_minggu[e->tanggal] = minggu_saat_ini;
    }

    int temp_shift_mingguan[MAX_ENTRIES][5] = {0};
    for (int i = 0; i < total_shift_entries; i++) {
        ShiftEntry *e = &shift_data[i];
        int idx = find_doctor_index(e->nama);
        if (idx == -1) continue;

        DoctorPref *pref = &doctor_data[idx];
        int minggu = tanggal_ke_minggu[e->tanggal] - 1;
        temp_shift_mingguan[idx][minggu]++;

        if (strcmp(e->shift, pref->preferensi_shift) != 0) {
            g_string_append_printf(output, "[%d] %s - Shift tidak sesuai preferensi (jadwal: %s, preferensi: %s)\n",
                                   e->tanggal, e->nama, e->shift, pref->preferensi_shift);
        }

        if (!is_waktu_sesuai(pref->preferensi_waktu, e->tanggal)) {
            g_string_append_printf(output, "[%d] %s - Waktu tidak sesuai preferensi (jadwal tanggal: %d, preferensi: %s bulan)\n",
                                   e->tanggal, e->nama, e->tanggal, pref->preferensi_waktu);
        }

        if (temp_shift_mingguan[idx][minggu] > pref->max_shift_per_minggu) {
            g_string_append_printf(output, "[%d] %s - Melebihi shift mingguan maksimum (%d per minggu)\n",
                                   e->tanggal, e->nama, pref->max_shift_per_minggu);
        }
    }

    if (output->len == 0) {
        g_string_append(output, "Tidak ada pelanggaran jadwal.\n");
    }

    gtk_text_buffer_set_text(buffer, output->str, -1);
    g_string_free(output, TRUE);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

    GtkWidget *close = create_styled_button("Tutup", G_CALLBACK(gtk_widget_destroy), window);
    gtk_box_pack_start(GTK_BOX(vbox), close, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
    set_status_message("Menampilkan pelanggaran jadwal");
}

// Callback functions untuk menu utama
static void on_doctor_management_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_doctor_management();
}

static void on_scheduling_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_scheduling();
}

static void on_schedule_view_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_schedule_view();
}

static void on_performance_report_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_performance_report();
}

static void on_exit_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    save_data_to_csv(DOKTER_CSV_PATH);
    free_memory();
    gtk_main_quit();
}

// Callback untuk kembali ke menu utama
static void on_back_to_main_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_main_menu();
}

// Doctor Management callbacks
static void on_add_doctor_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_add_doctor_dialog(NULL, NULL);
}

static void on_delete_doctor_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_delete_doctor_dialog(NULL, NULL);
}

static void on_search_doctor_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_search_doctor_dialog(NULL, NULL);
}

static void on_show_all_doctors_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_all_doctors(NULL, NULL);
}

static void on_undo_last_action_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_undo_dialog(NULL, NULL);
}

static void on_show_activity_log_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_activity_log(NULL, NULL);
}

// Scheduling callback
static void on_generate_schedule_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
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
    (void)widget; (void)data;
    show_daily_schedule_dialog(NULL, NULL);
}

static void on_weekly_schedule_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_weekly_schedule_dialog(NULL, NULL);
}

static void on_monthly_schedule_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_monthly_schedule(NULL, NULL);
}

// Performance report callbacks
static void on_generate_report_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
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
    (void)widget; (void)data;
    show_shift_totals_dialog(NULL, NULL);
}

static void on_view_violations_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
    show_violations_view(NULL, NULL);
}

// Function untuk membuat main menu
static void show_main_menu(void) {
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
    
    GtkWidget *title = gtk_label_new("Sistem Manajemen Jadwal Dokter Rumah Sakit Sehat Selalu");
    gtk_widget_set_name(title, "title-label");
    
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
    (void)widget; (void)event; (void)data;
    on_exit_clicked(NULL, NULL);
    return FALSE;
}

// Main GUI function
int gui_main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    load_data_dari_csv(DOKTER_CSV_PATH);
    
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Sistem Manajemen Jadwal Dokter");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 600, 500);
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), FALSE);
    
    g_signal_connect(main_window, "delete-event", G_CALLBACK(on_window_delete_event), NULL);
    
    GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), main_container);
    
    main_stack = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_container), main_stack, TRUE, TRUE, 0);
    
    status_label = gtk_label_new("Memuat aplikasi...");
    gtk_widget_set_margin_start(status_label, 10);
    gtk_widget_set_margin_end(status_label, 10);
    gtk_widget_set_margin_top(status_label, 5);
    gtk_widget_set_margin_bottom(status_label, 5);
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_container), separator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_container), status_label, FALSE, FALSE, 0);
    
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
        "}",
        -1, NULL);
    
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    
    show_main_menu();
    
    gtk_widget_show_all(main_window);
    
    gtk_main();
    
    return 0;
}