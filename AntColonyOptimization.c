#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_LEN_STRING 255
#define MAX_CITIES 100
#define M_PI 3.14159265358979323846
#define ALPHA 1.0
#define BETA 2.0
#define RHO 0.5
#define Q 100.0
#define NUM_ANTS 10
#define MAX_ITERATIONS 100

typedef struct {
    int tour[MAX_CITIES];
    double tour_length;
} Ant;

// Definisi struct Node untuk menyimpan data kota
typedef struct Node {
    char nama_kota[MAX_LEN_STRING];
    double lintang;
    double bujur;
    struct Node* next;
} Node;

// Fungsi untuk menambahkan node baru ke linked list
int add(Node **head, double data_lintang, double data_bujur, char nama[]) {
    Node *temp;
    temp = (Node*)malloc(sizeof(Node));

    temp->bujur = data_bujur;
    temp->lintang = data_lintang;
    strcpy(temp->nama_kota, nama);

    if (*head == NULL) {
        temp->next = NULL;
        *head = temp;
    } else {
        temp->next = *head;
        *head = temp;
    }
    return 0;
}

// Fungsi untuk membaca data kota dari file dan membentuk linked list
Node* input_file() {
    Node *Linked_list_kota = NULL;

    char file_name[MAX_LEN_STRING];
    printf("Masukkan File Map: ");
    scanf("%s", file_name);

    FILE* stream = fopen(file_name, "r");

    if (stream == NULL) {
        printf("File tidak ditemukan\n");
        return NULL;
    }

    char line[MAX_LEN_STRING];
    char tempLine[MAX_LEN_STRING];
    char token_bujur[MAX_LEN_STRING];
    char token_lintang[MAX_LEN_STRING];
    char kota_temp[MAX_LEN_STRING];
    double lintang, bujur;

    // Membaca setiap baris dari file
    while (fgets(line, MAX_LEN_STRING, stream)) {
        strcpy(tempLine, line);
        strcpy(kota_temp, strtok(tempLine, ","));
        strcpy(token_lintang, strtok(NULL, ","));
        strcpy(token_bujur, strtok(NULL, "\n"));
        sscanf(token_bujur, "%lf", &bujur);
        sscanf(token_lintang, "%lf", &lintang);
        add(&Linked_list_kota, lintang, bujur, kota_temp);
    }

    fclose(stream);
    return Linked_list_kota;
}

// Fungsi untuk menghitung jarak Haversine antara dua titik (lintang dan bujur)
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

// Fungsi untuk menghitung jumlah kota
int calculate_cities(Node *daftar_kota){
    int jumlah_kota = 0;
    Node *temp = daftar_kota;
    while (temp != NULL) {
        jumlah_kota++;
        temp = temp->next;
    }

    return jumlah_kota;
}

// Fungsi untuk membuat array node kota dari linked list
void make_cities_arrOfNode(Node *daftar_kota, Node *cities[], int jumlah_kota) {
    Node *temp = daftar_kota;
    for (int i = 0; i < jumlah_kota; i++) {
        cities[i] = temp;
        temp = temp->next;
    }
}

// Fungsi untuk membuat matriks jarak antar kota
void make_distanceMatrices(Node *cities[], int jumlah_kota, double distances[jumlah_kota][jumlah_kota]) {
    for (int i = 0; i < jumlah_kota; i++) {
        for (int j = 0; j < jumlah_kota; j++) {
            if (i == j) {
                distances[i][j] = 0;
            } else {
                distances[i][j] = haversine(cities[i]->lintang, cities[i]->bujur, cities[j]->lintang, cities[j]->bujur);
            }
        }
    }
}

// Fungsi untuk menemukan indeks kota berdasarkan nama kota
int find_city_index(Node* cities[], int numCities, char* cityName) {
    for (int i = 0; i < numCities; i++) {
        if (strcmp(cities[i]->nama_kota, cityName) == 0) {
            return i;
        }
    }
    return -1;
}

// Fungsi untuk mencetak rute terbaik
void print_bestRoute(Node *cities[], int bestPath[], int jumlah_kota, double minCost) {
    printf("Best route found:\n");
    for (int i = 0; i <= jumlah_kota; i++) {
        printf("%s", cities[bestPath[i]]->nama_kota);
        if (i < jumlah_kota) {
            printf(" -> ");
        }
    }
    printf("\nBest route distance: %lf km\n", minCost);
}

// Inisialisasi semut
void initialize_ants(Ant ants[], int numAnts, int numCities) {
    for (int i = 0; i < numAnts; i++) {
        for (int j = 0; j < numCities; j++) {
            ants[i].tour[j] = -1;
        }
        ants[i].tour_length = 0.0;
    }
}

// Fungsi untuk menjalankan algoritma ACO
// Fungsi untuk menjalankan algoritma ACO
void run_ACO(Node *cities[], int numCities, double distances[numCities][numCities]) {
    Ant ants[NUM_ANTS];
    double pheromones[numCities][numCities];
    double best_tour_length = INFINITY;
    int best_tour[numCities];

    // Inisialisasi jejak feromon awal
    for (int i = 0; i < numCities; i++) {
        for (int j = 0; j < numCities; j++) {
            pheromones[i][j] = 1.0;
        }
    }

    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        // Inisialisasi semut
        initialize_ants(ants, NUM_ANTS, numCities);

        // Masing-masing semut membuat tur
        for (int ant = 0; ant < NUM_ANTS; ant++) {
            int start_city = rand() % numCities; // Memilih kota awal secara acak
            ants[ant].tour[0] = start_city;

            for (int i = 1; i < numCities; i++) {
                // Memilih kota berikutnya berdasarkan probabilitas
                int current_city = ants[ant].tour[i - 1];
                int next_city = -1;
                double roulette = (double)rand() / RAND_MAX;
                double total_prob = 0.0;

                for (int j = 0; j < numCities; j++) {
                    if (j != current_city && ants[ant].tour[j] == -1) {
                        double prob = pow(pheromones[current_city][j], ALPHA) * pow(1.0 / distances[current_city][j], BETA);
                        total_prob += prob;
                        if (roulette <= total_prob) {
                            next_city = j;
                            break;
                        }
                    }
                }
                ants[ant].tour[i] = next_city;
            }

            // Hitung panjang tur semut
            ants[ant].tour_length = 0.0;
            for (int i = 0; i < numCities - 1; i++) {
                ants[ant].tour_length += distances[ants[ant].tour[i]][ants[ant].tour[i + 1]];
            }
            ants[ant].tour_length += distances[ants[ant].tour[numCities - 1]][ants[ant].tour[0]]; // Kembali ke kota awal
        }

        // Memperbarui jejak feromon
        for (int i = 0; i < numCities; i++) {
            for (int j = 0; j < numCities; j++) {
                pheromones[i][j] *= (1.0 - RHO);
            }
        }

        // Memperbarui jejak feromon berdasarkan panjang tur terbaik
        for (int ant = 0; ant < NUM_ANTS; ant++) {
            if (ants[ant].tour_length < best_tour_length) {
                best_tour_length = ants[ant].tour_length;
                memcpy(best_tour, ants[ant].tour, sizeof(int) * numCities);
            }

            for (int i = 0; i < numCities - 1; i++) {
                pheromones[ants[ant].tour[i]][ants[ant].tour[i + 1]] += Q / ants[ant].tour_length;
                pheromones[ants[ant].tour[i + 1]][ants[ant].tour[i]] += Q / ants[ant].tour_length;
            }
            pheromones[ants[ant].tour[numCities - 1]][ants[ant].tour[0]] += Q / ants[ant].tour_length;
            pheromones[ants[ant].tour[0]][ants[ant].tour[numCities - 1]] += Q / ants[ant].tour_length;
        }
    }

    // Cetak tur terbaik
    printf("Best route found:\n");
    for (int i = 0; i < numCities; i++) {
        printf("%s", cities[best_tour[i]]->nama_kota);
        if (i < numCities - 1) {
            printf(" -> ");
        }
    }
    printf("\nBest route distance: %lf km\n", best_tour_length);
}


// Fungsi utama
int main(void) {
    Node *daftar_kota = input_file();

    if (daftar_kota == NULL) {
        return -1;
    }

    // Menghitung jumlah kota
    int numCities = calculate_cities(daftar_kota);
    
    // Membuat array node kota
    Node *cities[numCities];
    make_cities_arrOfNode(daftar_kota, cities, numCities);

    // Membuat matriks jarak antar kota
    double distances[MAX_LEN_STRING][MAX_LEN_STRING];
    make_distanceMatrices(cities, numCities, distances);

    // Implementasi algoritma ACO
    run_ACO(cities, numCities, distances);

    return 0;
}
