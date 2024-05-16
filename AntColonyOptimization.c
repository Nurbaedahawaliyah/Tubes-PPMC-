#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_LEN_STRING 255
#define MAX_CITIES 100
#define NUM_ANTS 10
#define ALPHA 1.0 // Pengaruh pheromone
#define BETA 2.0  // Pengaruh visibilitas (jarak)
#define RHO 0.1   // Tingkat penguapan pheromone
#define Q 100.0   // Jumlah pheromone yang dilepaskan oleh semut

// Definisi struct Node untuk menyimpan data kota
typedef struct Node {
    char nama_kota[MAX_LEN_STRING];
    double lintang;
    double bujur;
    struct Node* next;
} Node;

// Definisi struct Ant untuk menyimpan status semut
typedef struct Ant {
    int tour[MAX_CITIES]; // Tur semut
    int visited[MAX_CITIES]; // Status kunjungan
    double tour_length; // Panjang tur
} Ant;

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

// Fungsi untuk inisialisasi semut
void init_ants(Ant ants[], int numCities) {
    for (int i = 0; i < NUM_ANTS; i++) {
        memset(ants[i].visited, 0, sizeof(ants[i].visited));
        ants[i].tour_length = 0.0;
    }
}

// Fungsi untuk memilih kota berikutnya berdasarkan aturan ACO
int select_next_city(Ant *ant, int numCities, double pheromones[][MAX_CITIES], double distances[][MAX_CITIES], double alpha, double beta) {
    double total = 0.0;
    double probabilities[numCities];

    // Hitung total probabilitas
    for (int i = 0; i < numCities; i++) {
        if (ant->visited[i] == 0) {
            total += pow(pheromones[ant->tour[numCities - 1]][i], alpha) * pow(1.0 / distances[ant->tour[numCities - 1]][i], beta);
        }
    }

    // Hitung probabilitas untuk setiap kota yang belum dikunjungi
    for (int i = 0; i < numCities; i++) {
        if (ant->visited[i] == 0) {
            probabilities[i] = (pow(pheromones[ant->tour[numCities - 1]][i], alpha) * pow(1.0 / distances[ant->tour[numCities - 1]][i], beta)) / total;
        } else {
            probabilities[i] = 0.0;
        }
    }

    // Pilih kota berikutnya berdasarkan probabilitas
    double r = (double)rand() / RAND_MAX;
    double sum = 0.0;
    for (int i = 0; i < numCities; i++) {
        sum += probabilities[i];
        if (sum >= r) {
            return i;
        }
    }
    // Jika gagal memilih, kota terakhir yang dipilih
    for (int i = 0; i < numCities; i++) {
        if (ant->visited[i] == 0) {
            return i;
        }
    }
    return -1; // Jika semua kota telah dikunjungi
}

// Fungsi untuk menemukan rute terpendek dengan menggunakan algoritma ACO
void find_shortest_path(Node *cities[], int numCities, double distances[][MAX_CITIES], double pheromones[][MAX_CITIES]) {
    srand(time(NULL)); // Inisialisasi seed untuk random number generator

    Ant ants[NUM_ANTS];
    double best_tour_length = INFINITY;
    int best_tour[MAX_CITIES];

    for (int iteration = 0; iteration < 1000; iteration++) {
        init_ants(ants, numCities);

        // Setiap semut memilih kota secara berurutan
        for (int i = 0; i < numCities - 1; i++) {
            for (int j = 0; j < NUM_ANTS; j++) {
                int next_city = select_next_city(&ants[j], numCities, pheromones, distances, ALPHA, BETA);
                ants[j].visited[next_city] = 1;
                ants[j].tour[i + 1] = next_city;
                ants[j].tour_length += distances[ants[j].tour[i]][next_city];
            }
        }

        // Update pheromones
        for (int i = 0; i < NUM_ANTS; i++) {
            ants[i].tour_length += distances[ants[i].tour[numCities - 1]][ants[i].tour[0]]; // Kembali ke kota awal
            if (ants[i].tour_length < best_tour_length) {
                best_tour_length = ants[i].tour_length;
                memcpy(best_tour, ants[i].tour, sizeof(best_tour));
            }
            for (int j = 0; j < numCities - 1; j++) {
                pheromones[ants[i].tour[j]][ants[i].tour[j + 1]] += Q / ants[i].tour_length;
                pheromones[ants[i].tour[j + 1]][ants[i].tour[j]] += Q / ants[i].tour_length;
            }
        }

        // Penguapan pheromone
        for (int i = 0; i < numCities; i++) {
            for (int j = 0; j < numCities; j++) {
                pheromones[i][j] *= (1.0 - RHO);
            }
        }
    }

    // Print best route
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
    double distances[MAX_CITIES][MAX_CITIES];
    make_distanceMatrices(cities, numCities, distances);

    // Matriks pheromones untuk ACO
    double pheromones[MAX_CITIES][MAX_CITIES];
    for (int i = 0; i < numCities; i++) {
        for (int j = 0; j < numCities; j++) {
            pheromones[i][j] = 0.01; // Inisialisasi dengan nilai kecil
        }
    }

    // Temukan rute terpendek dengan ACO
    find_shortest_path(cities, numCities, distances, pheromones);

    return 0;
}
