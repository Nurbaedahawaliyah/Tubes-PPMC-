#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

#define MAX_LEN_STRING 255
#define EARTH_RADIUS 6371.0
#define M_PI 3.14159265358979323846

// Structure to hold city information
typedef struct Node {
    char nama_kota[MAX_LEN_STRING];
    double bujur;
    double lintang;
    struct Node* next;
} Node;

int add(Node **head, double data_lintang, double data_bujur, char nama[]) {
    Node *temp = (Node*)malloc(sizeof(Node));

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

void print(Node* hasil) {
    Node* display = hasil;
    while (display != NULL) {
        printf("%s \t: %f\t|| %f\n", display->nama_kota, display->lintang, display->bujur);
        display = display->next;
    }
}

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
    while (fgets(line, MAX_LEN_STRING, stream)) {
        char kota_temp[MAX_LEN_STRING];
        double lintang, bujur;

        sscanf(line, "%[^,],%lf,%lf", kota_temp, &lintang, &bujur);
        add(&Linked_list_kota, lintang, bujur, kota_temp);
    }

    fclose(stream);
    return Linked_list_kota;
}

// Convert degrees to radians
double toRadians(double degree) {
    return degree * M_PI / 180.0;
}

// Calculate distance between two points using the haversine formula
double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(lon2 - lon1);
    lat1 = toRadians(lat1);
    lat2 = toRadians(lat2);

    double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
    double c = 2 * asin(sqrt(a));
    return EARTH_RADIUS * c;
}

// Find the nearest unvisited city
int findNearestCity(Node cities[], int currentCity, int n, int visited[]) {
    int nearestCity = -1;
    double minDistance = DBL_MAX;
    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            double distance = haversine(cities[currentCity].lintang, cities[currentCity].bujur,
                                        cities[i].lintang, cities[i].bujur);
            if (distance < minDistance) {
                minDistance = distance;
                nearestCity = i;
            }
        }
    }
    return nearestCity;
}

// Solve the TSP problem using a greedy approach
double solveTSP(Node cities[], int n, int startCity, int *route) {
    int *visited = (int *)calloc(n, sizeof(int));
    double totalDistance = 0.0;

    int currentCity = startCity;
    visited[currentCity] = 1;
    route[0] = currentCity;

    for (int i = 1; i < n; i++) {
        int nextCity = findNearestCity(cities, currentCity, n, visited);
        if (nextCity == -1) break;
        totalDistance += haversine(cities[currentCity].lintang, cities[currentCity].bujur,
                                   cities[nextCity].lintang, cities[nextCity].bujur);
        currentCity = nextCity;
        visited[currentCity] = 1;
        route[i] = currentCity;
    }

    totalDistance += haversine(cities[currentCity].lintang, cities[currentCity].bujur,
                               cities[startCity].lintang, cities[startCity].bujur);

    free(visited);
    return totalDistance;
}

// Find the best starting city for TSP
void findBestStartingCity(Node cities[], int n, int *bestCity, double *bestDistance, int *bestRoute) {
    double minDistance = DBL_MAX;
    int *currentRoute = (int *)malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        double currentDistance = solveTSP(cities, n, i, currentRoute);
        if (currentDistance < minDistance) {
            minDistance = currentDistance;
            *bestCity = i;
            *bestDistance = currentDistance;
            memcpy(bestRoute, currentRoute, n * sizeof(int));
        }
    }

    free(currentRoute);
}

// Write the route to an output file
void writeRouteToFile(Node cities[], int n, int startCity, double totalDistance, int *route, FILE *outputFile) {
    fprintf(outputFile, "Best starting city: %s\n", cities[startCity].nama_kota);
    fprintf(outputFile, "Shortest route: ");
    for (int i = 0; i < n; i++) {
        fprintf(outputFile, "%s -> ", cities[route[i]].nama_kota);
    }
    fprintf(outputFile, "%s\n", cities[startCity].nama_kota);
    fprintf(outputFile, "Total distance: %.2f km\n", totalDistance);
}

// Main function
int main() {
    Node *cities_list = input_file();
    if (cities_list == NULL) return 1;

    int n = 0;
    Node *temp = cities_list;
    while (temp != NULL) {
        n++;
        temp = temp->next;
    }

    Node *cities = (Node *)malloc(n * sizeof(Node));
    temp = cities_list;
    for (int i = 0; i < n; i++) {
        strcpy(cities[i].nama_kota, temp->nama_kota);
        cities[i].lintang = temp->lintang;
        cities[i].bujur = temp->bujur;
        temp = temp->next;
    }

    char startCityName[MAX_LEN_STRING];
    printf("Masukkan kota pertama yang ingin dikunjungi: ");
    scanf("%s", startCityName);

    int startCityIndex = -1;
    for (int i = 0; i < n; i++) {
        if (strcmp(cities[i].nama_kota, startCityName) == 0) {
            startCityIndex = i;
            break;
        }
    }

    if (startCityIndex == -1) {
        printf("Kota pertama yang dimasukkan tidak ditemukan dalam list.\n");
        free(cities);
        return 1;
    }

    int bestCity;
    double bestDistance;
    int *bestRoute = (int *)malloc(n * sizeof(int));

    findBestStartingCity(cities, n, &bestCity, &bestDistance, bestRoute);

    FILE *outputFile = fopen("output.txt", "w");
    if (outputFile == NULL) {
        printf("Tidak dapat membuka file output.\n");
        free(cities);
        free(bestRoute);
        return 1;
    }

    writeRouteToFile(cities, n, bestCity, bestDistance, bestRoute, outputFile);

    fclose(outputFile);
    free(cities);
    free(bestRoute);

    return 0;
}
