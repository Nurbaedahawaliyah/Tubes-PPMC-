// source: https://www.geeksforgeeks.org/haversine-formula-to-find-distance-between-two-points-on-a-sphere/

#include <stdio.h>
#include <math.h>


#define M_PI 3.14159265358979323846

// Fungsi untuk menghitung jarak antara dua titik koordinat menggunakan rumus haversine
double haversine(double lat1, double lon1, double lat2, double lon2) {
    // Perbedaan lintang dan bujur antara kedua titik
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    // Konversi sudut lintang ke radian
    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    // Rumus haversine
    double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
    double rad = 6371; // Radius bumi dalam kilometer
    double c = 2 * asin(sqrt(a));

    // Jarak antara dua titik dalam kilometer
    return rad * c;
}

// Debugging
// int main() {
//     double lat1 = 51.5007; // Koordinat lintang untuk titik pertama (dalam derajat)
//     double lon1 = 0.1246;  // Koordinat bujur untuk titik pertama (dalam derajat)
//     double lat2 = 40.6892; // Koordinat lintang untuk titik kedua (dalam derajat)
//     double lon2 = 74.0445; // Koordinat bujur untuk titik kedua (dalam derajat)
    
//     // Memanggil fungsi haversine untuk menghitung jarak antara dua titik
//     double distance = haversine(lat1, lon1, lat2, lon2);
    
//     // Menampilkan hasil jarak dalam kilometer
//     printf("Jarak antara kedua titik adalah: %.2f KM\n", distance);

//     return 0;
// }

