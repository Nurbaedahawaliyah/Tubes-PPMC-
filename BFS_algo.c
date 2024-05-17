#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

#define EARTH_RADIUS 6371.0 // Radius of Earth in kilometers
#define MAX_LEN_STRING 255
#define MAX_CITIES 15

typedef struct Node {
    char nama_kota[MAX_LEN_STRING];
    double lintang;
    double bujur;
    struct Node* next;
} Node;

typedef struct {
    int path[MAX_CITIES];
    double cost;
} PathNode;

double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    double a = sin(dLat / 2) * sin(dLat / 2) + sin(dLon / 2) * sin(dLon / 2) * cos(lat1) * cos(lat2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RADIUS * c;
}

double calculateTotalDistance(int *path, Node *cities[], int numCities) {
    double totalDistance = 0;
    for (int i = 0; i < numCities; ++i) {
        totalDistance += haversine(cities[path[i]]->lintang, cities[path[i]]->bujur, 
                                   cities[path[(i + 1) % numCities]]->lintang, cities[path[(i + 1) % numCities]]->bujur);
    }
    return totalDistance;
}

int findCityIndex(Node *cities[], int numCities, char *cityName) {
    for (int i = 0; i < numCities; ++i) {
        if (strcmp(cities[i]->nama_kota, cityName) == 0) {
            return i;
        }
    }
    return -1;
}

void bfsTSP(Node *cities[], int numCities, int startCityIndex) {
    int queueSize = 1;
    int maxQueueSize = 1 << (numCities - 1); // 2^(numCities-1)
    PathNode *queue = (PathNode *)malloc(maxQueueSize * sizeof(PathNode));
    PathNode bestPath;
    bestPath.cost = DBL_MAX;

    for (int i = 0; i < numCities; ++i) {
        queue[0].path[i] = -1;
    }
    queue[0].path[0] = startCityIndex;
    queue[0].cost = 0;

    while (queueSize > 0) {
        PathNode current = queue[--queueSize];

        int depth = 0;
        while (depth < numCities && current.path[depth] != -1) ++depth;

        if (depth == numCities) {
            double cost = calculateTotalDistance(current.path, cities, numCities);
            if (cost < bestPath.cost) {
                bestPath = current;
                bestPath.cost = cost;
            }
            continue;
        }

        for (int i = 0; i < numCities; ++i) {
            int found = 0;
            for (int j = 0; j < depth; ++j) {
                if (current.path[j] == i) {
                    found = 1;
                    break;
                }
            }
            if (found) continue;

            PathNode newNode = current;
            newNode.path[depth] = i;
            newNode.cost = current.cost + haversine(cities[current.path[depth - 1]]->lintang, cities[current.path[depth - 1]]->bujur, cities[i]->lintang, cities[i]->bujur);
            queue[queueSize++] = newNode;
        }
    }

    printf("Shortest route: ");
    for (int i = 0; i < numCities; ++i) {
        printf("%s -> ", cities[bestPath.path[i]]->nama_kota);
    }
    printf("%s\n", cities[bestPath.path[0]]->nama_kota);
    printf("Total distance: %.2f km\n", bestPath.cost);

    free(queue);
}

int main() {
    Node cityBandung = {"Bandung", -6.917464, 107.619123, NULL};
    Node cityJakarta = {"Jakarta", -6.208763, 106.845599, NULL};
    Node citySurabaya = {"Surabaya", -7.257472, 112.752088, NULL};
    Node citySemarang = {"Semarang", -6.966667, 110.416667, NULL};
    Node cityBali = {"Bali", -8.340539, 115.091949, NULL};
    Node cityYogyakarta = {"Yogyakarta", -7.797068, 110.370529, NULL};

    Node *cities[] = {&cityBandung, &cityJakarta, &citySurabaya, &citySemarang, &cityBali, &cityYogyakarta};
    int numCities = sizeof(cities) / sizeof(cities[0]);
    
    char startCity[MAX_LEN_STRING];
    printf("Enter the starting city: ");
    scanf("%s", startCity);

    int startCityIndex = findCityIndex(cities, numCities, startCity);
    if (startCityIndex == -1) {
        printf("City not found in the list.\n");
        return 1;
    }
    
    bfsTSP(cities, numCities, startCityIndex);

    return 0;
}
