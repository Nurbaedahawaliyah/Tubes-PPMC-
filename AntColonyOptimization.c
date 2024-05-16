
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "struct.h"
#include "haversine.h"
#include "input.h"

#define MAX_LEN_STRING 255
#define M_PI 3.14159265358979323846
#define ALPHA 1.0 // Parameter Alpha untuk pengaruh feromon
#define BETA 2.0  // Parameter Beta untuk pengaruh visibilitas
#define RHO 0.5   // Tingkat evaporasi
#define Q 100     // Kuantitas feromon

// Definisi Struktur Kota untuk menyimpan data kota
typedef struct {
    char nama_kota[MAX_LEN_STRING];
    double lintang;
    double bujur;
} Kota;

// Definisi Struktur Ant
typedef struct {
    int *rute;       // Rute semut
    int *dikunjungi; // Array untuk melacak kota yang telah dikunjungi
    double panjang_rute; // Panjang rute
} Ant;

// Fungsi untuk menghitung jarak antara dua kota menggunakan rumus Haversine
double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
    double rad = 6371; // Radius bumi dalam kilometer
    double c = 2 * asin(sqrt(a));

    return rad * c;
}

// Fungsi untuk inisialisasi semut
Ant *inisialisasi_ant(int jumlah_kota) {
    Ant *ant = (Ant *)malloc(jumlah_kota * sizeof(Ant));
    for (int i = 0; i < jumlah_kota; i++) {
        ant[i].rute = (int *)malloc(jumlah_kota * sizeof(int));
        ant[i].dikunjungi = (int *)malloc(jumlah_kota * sizeof(int));
        memset(ant[i].dikunjungi, 0, jumlah_kota * sizeof(int)); // Menandai semua kota sebagai belum dikunjungi
        ant[i].panjang_rute = 0.0;
    }
    return ant;
}

// Fungsi untuk membebaskan memori yang digunakan oleh semut
void bebaskan_ant(Ant *ant, int jumlah_kota) {
    for (int i = 0; i < jumlah_kota; i++) {
        free(ant[i].rute);
        free(ant[i].dikunjungi);
    }
    free(ant);
}

// Fungsi untuk memilih kota selanjutnya untuk ant berdasarkan feromon dan visibilitas
int pilih_kota_selanjutnya(Ant *ant, double **feromon, double **visibilitas, int kota_sekarang, int jumlah_kota) {
    double *probabilitas = (double *)malloc(jumlah_kota * sizeof(double));
    double total = 0.0;

    // Menghitung total probabilitas untuk semua kota yang belum dikunjungi
    for (int i = 0; i < jumlah_kota; i++) {
        if (!ant->dikunjungi[i]) {
            probabilitas[i] = pow(feromon[kota_sekarang][i], ALPHA) * pow(visibilitas[kota_sekarang][i], BETA);
            total += probabilitas[i];
        } else {
            probabilitas[i] = 0.0; // Kota sudah dikunjungi, probabilitas nol
        }
    }

    // Memilih kota selanjutnya berdasarkan probabilitas yang dihitung
    double rnd = (double)rand() / RAND_MAX; // Angka acak antara 0 dan 1
    double kumulatif = 0.0;
    for (int i = 0; i < jumlah_kota; i++) {
        if (!ant->dikunjungi[i]) {
            probabilitas[i] /= total; // Normalisasi probabilitas
            kumulatif += probabilitas[i];
            if (rnd <= kumulatif) {
                free(probabilitas);
                return i;
            }
        }
    }

    // Jika probabilitas yang dipilih tidak cocok dengan tepat karena presisi titik mengambang,
    // memilih kota dengan probabilitas tertinggi
    int indeks_maks = -1;
    double probabilitas_maks = -1.0;
    for (int i = 0; i < jumlah_kota; i++) {
        if (!ant->dikunjungi[i] && probabilitas[i] > probabilitas_maks) {
            probabilitas_maks = probabilitas[i];
            indeks_maks = i;
        }
    }

    free(probabilitas);
    return indeks_maks;
}

// Fungsi untuk memperbarui tingkat feromon
void perbarui_feromon(double **feromon, Ant *ant, int jumlah_kota) {
    // Mengurangi tingkat feromon di semua rute
    for (int i = 0; i < jumlah_kota; i++) {
        for (int j = 0; j < jumlah_kota; j++) {
            feromon[i][j] *= (1 - RHO);
        }
    }

    // Menambahkan tingkat feromon berdasarkan panjang rute ant
    for (int k = 0; k < jumlah_kota; k++) {
        for (int i = 0; i < jumlah_kota - 1; i++) {
            int kota1 = ant[k].rute[i];
            int kota2 = ant[k].rute[i + 1];
            feromon[kota1][kota2] += Q / ant[k].panjang_rute;
            feromon[kota2][kota1] += Q / ant[k].panjang_rute; // Feromon simetris
        }
    }
}

// Fungsi untuk mencari rute terbaik dari semua ant
void cari_rute_terbaik(Ant *ant, int jumlah_kota) {
    double panjang_terbaik = ant[0].panjang_rute;
    int indeks_terbaik = 0;

    // Mencari ant dengan panjang rute terbaik
    for (int i = 1; i < jumlah_kota; i++) {
        if (ant[i].panjang_rute < panjang_terbaik) {
            panjang_terbaik = ant[i].panjang_rute;
            indeks_terbaik = i;
        }
    }

    // Salin rute terbaik ke ant pertama
    memcpy(ant[0].rute, ant[indeks_terbaik].rute, jumlah_kota * sizeof(int));
    ant[0].panjang_rute = panjang_terbaik;
}

// Fungsi untuk melakukan Optimasi Koloni Semut untuk TSP
void optimasi_koloni_semut(Kota *kota, int jumlah_kota, char *kota_awal) {
    // Inisialisasi feromon dan visibilitas
    double **feromon = (double **)malloc(jumlah_kota * sizeof(double *));
    double **visibilitas = (double **)malloc(jumlah_kota * sizeof(double *));
    for (int i = 0; i < jumlah_kota; i++) {
        feromon[i] = (double *)malloc(jumlah_kota * sizeof(double));
        visibilitas[i] = (double *)malloc(jumlah_kota * sizeof(double));
        for (int j = 0; j < jumlah_kota; j++) {
            feromon[i][j] = 1.0;                       // Inisialisasi feromon menjadi 1.0
            visibilitas[i][j] = 1.0 / haversine(kota[i].lintang, kota[i].bujur, kota[j].lintang, kota[j].bujur); // Inisialisasi visibilitas menjadi invers dari jarak
        }
    }

    // Inisialisasi ant
    Ant *ant = inisialisasi_ant(jumlah_kota);

    // Temukan indeks kota awal
    int indeks_awal = -1;
    for (int i = 0; i < jumlah_kota; i++) {
        if (strcmp(kota[i].nama_kota, kota_awal) == 0) {
            indeks_awal = i;
            break;
        }
    }

    // Lakukan iterasi
    int iterasi_maks = 100;
    for (int iterasi = 0; iterasi < iterasi_maks; iterasi++) {
        // Reset ant
        for (int i = 0; i < jumlah_kota; i++) {
            memset(ant[i].dikunjungi, 0, jumlah_kota * sizeof(int)); // Tandai semua kota sebagai belum dikunjungi
            ant[i].panjang_rute = 0.0;
        }

        // Konstruksi rute
        for (int k = 0; k < jumlah_kota; k++) {
            for (int i = 0; i < jumlah_kota - 1; i++) {
                int kota_sekarang = ant[k].rute[i];
                int kota_selanjutnya = pilih_kota_selanjutnya(&ant[k], feromon, visibilitas, kota_sekarang, jumlah_kota);
                ant[k].rute[i + 1] = kota_selanjutnya;
                ant[k].dikunjungi[kota_selanjutnya] = 1;
                ant[k].panjang_rute += haversine(kota[kota_sekarang].lintang, kota[kota_sekarang].bujur, kota[kota_selanjutnya].lintang, kota[kota_selanjutnya].bujur);
            }
            // Lengkapi rute dengan kembali ke kota awal
            ant[k].rute[jumlah_kota] = indeks_awal;
            ant[k].panjang_rute += haversine(kota[ant[k].rute[jumlah_kota - 1]].lintang, kota[ant[k].rute[jumlah_kota - 1]].bujur, kota[indeks_awal].lintang, kota[indeks_awal].bujur);
        }

        // Perbarui feromon
        perbarui_feromon(feromon, ant, jumlah_kota);

        // Temukan rute terbaik
        cari_rute_terbaik(ant, jumlah_kota);
    }

    // Cetak rute terbaik
    printf("Best route found:\n");
    for (int i = 0; i <= jumlah_kota; i++) {
        printf("%s", kota[ant[0].rute[i]].nama_kota);
        if (i < jumlah_kota) {
            printf(" -> ");
        }
    }
    printf("\nBest route distance: %.5lf km\n", ant[0].panjang_rute);

    // Bebaskan memori
    for (int i = 0; i < jumlah_kota; i++) {
        free(feromon[i]);
        free(visibilitas[i]);
    }
    free(feromon);
    free(visibilitas);
    bebaskan_ant(ant, jumlah_kota);
}

// Fungsi utama
int main() {
    // Baca kota dari file
    char nama_file[MAX_LEN_STRING];
    printf("Enter list of cities file name: ");
    scanf("%s", nama_file);

    FILE *file = fopen(nama_file, "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    int jumlah_kota;
    fscanf(file, "%d", &jumlah_kota);
    Kota *kota = (Kota *)malloc(jumlah_kota * sizeof(Kota));
    for (int i = 0; i < jumlah_kota; i++) {
        fscanf(file, "%s %lf %lf", kota[i].nama_kota, &kota[i].lintang, &kota[i].bujur);
    }
    fclose(file);

    // Input kota awal dari pengguna
    char kota_awal[MAX_LEN_STRING];
    printf("Enter starting point: ");
    scanf("%s", kota_awal);

    // Optimasi Koloni Semut
    optimasi_koloni_semut(kota, jumlah_kota, kota_awal);

    // Bebaskan memori
    free(kota);

    return 0;
}
