#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_LEN_STRING 255
#define MAX_CITIES 100 // Batas maksimum jumlah kota
#define M_PI 3.14159265358979323846

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

// Fungsi untuk menemukan indeks kota berdasarkan nama kota
int find_city_index(Node* cities[], int numCities, char* cityName) {
    for (int i = 0; i < numCities; i++) {
        if (strcmp(cities[i]->nama_kota, cityName) == 0) {
            return i;
        }
    }
    return -1;
}

// Fungsi untuk memecahkan masalah TSP menggunakan algoritma Branch and Bound
void branch_and_bound(Node* cities[], int numCities, int visited[], int currPos, int count, double cost, double* minCost, double distances[MAX_CITIES][MAX_CITIES], int path[], int bestPath[], int startIndex) {
    if (count == numCities) {
        double returnCost = distances[currPos][startIndex];
        if (returnCost > 0 && (cost + returnCost < *minCost)) {
            *minCost = cost + returnCost;
            for (int i = 0; i < numCities; i++) {
                bestPath[i] = path[i];
            }
            bestPath[numCities] = startIndex; // Kembali ke kota awal
        }
        return;
    }

    for (int i = 0; i < numCities; i++) {
        if (!visited[i] && distances[currPos][i] > 0) {
            double newCost = cost + distances[currPos][i];
            visited[i] = 1;
            path[count] = i;
            branch_and_bound(cities, numCities, visited, i, count + 1, newCost, minCost, distances, path, bestPath, startIndex);
            visited[i] = 0;
        }
    }
}

// Fungsi utama
int main(void) {
    Node *daftar_kota = input_file();

    if (daftar_kota == NULL) {
        return -1;
    }

    // Menghitung jumlah kota
    int numCities = 0;
    Node *temp = daftar_kota;
    while (temp != NULL) {
        numCities++;
        temp = temp->next;
    }

    if (numCities > MAX_CITIES) {
        printf("Jumlah kota melebihi batas maksimum (%d)\n", MAX_CITIES);
        return -1;
    }

    // Membuat array node kota
    Node* cities[numCities];
    temp = daftar_kota;
    for (int i = 0; i < numCities; i++) {
        cities[i] = temp;
        temp = temp->next;
    }

    // Membuat matriks jarak antar kota
    double distances[MAX_CITIES][MAX_CITIES];
    for (int i = 0; i < numCities; i++) {
        for (int j = 0; j < numCities; j++) {
            if (i == j) {
                distances[i][j] = 0;
            } else {
                distances[i][j] = haversine(cities[i]->lintang, cities[i]->bujur, cities[j]->lintang, cities[j]->bujur);
            }
        }
    }

    // Handling case jika kota start tidak ada
    char startingCity[MAX_LEN_STRING];
    printf("Enter starting point: ");
    scanf("%s", startingCity);

    int startIndex = find_city_index(cities, numCities, startingCity);
    if (startIndex == -1) {
        printf("Starting city not found\n");
        return 0;
    }

    // Jika ada, maka jalankan algoritma branch and bound dan hitung waktu eksekusi
    clock_t start_time = clock();

    // Inisialisasi array visited
    int visited[MAX_CITIES];
    memset(visited, 0, sizeof(visited));
    visited[startIndex] = 1; // Mulai dari kota yang dipilih
    int path[MAX_CITIES + 1];  // Untuk menyimpan jalur saat ini
    int bestPath[MAX_CITIES + 1];  // Untuk menyimpan jalur terbaik
    path[0] = startIndex;

    // Mulai branch and bound untuk mencari biaya minimum
    double minCost = INFINITY;
    branch_and_bound(cities, numCities, visited, startIndex, 1, 0, &minCost, distances, path, bestPath, startIndex);

    // Hitung waktu eksekusi
    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Cetak rute terbaik
    printf("Best route found:\n");
    for (int i = 0; i <= numCities; i++) {
        printf("%s", cities[bestPath[i]]->nama_kota);
        if (i < numCities) {
            printf(" -> ");
        }
    }
    printf("\nBest route distance: %lf km\n", minCost);

    printf("Time elapsed: %.10f s\n", time_spent);

    return 0;
}
