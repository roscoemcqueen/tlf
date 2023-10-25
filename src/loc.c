#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <math.h>

#define MAX_BUFFER_SIZE 1024

char *calculateMaidenheadGridSquare(const char *latitude, const char *longitude) {
    const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    double lat = atof(latitude);
    double lon = atof(longitude);

    int lat_index = (int)((90 + lat) / 10);
    int lon_index = (int)((180 + lon) / 20);

    char grid_square[7];
    grid_square[0] = alphabet[lon_index];
    grid_square[1] = alphabet[lat_index];

    double lat_remainder = fmod(90 + lat, 10);
    double lon_remainder = fmod(180 + lon, 20);
    snprintf(grid_square + 2, 5, "%02d%02d", (int)lon_remainder, (int)lat_remainder);

    return strdup(grid_square);
}

size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char *buffer = (char *)userp;
    strncat(buffer, (char *)contents, realsize);
    return realsize;
}

int main() {
    char aprs_call[10]; // Adjust the size as needed
    char aprs_api[256];

    FILE *aprs_dat_file = fopen("aprs.dat", "r");
    if (aprs_dat_file) {
        char line[256];
        while (fgets(line, sizeof(line), aprs_dat_file)) {
            if (strstr(line, "APRS_CALL")) {
                char *equal_sign = strchr(line, '=');
                if (equal_sign) {
                    strncpy(aprs_call, equal_sign + 1, sizeof(aprs_call));
                    aprs_call[strcspn(aprs_call, "\n")] = '\0'; // Remove the newline character
                }
            } else if (strstr(line, "APRS_API")) {
                char *equal_sign = strchr(line, '=');
                if (equal_sign) {
                    strncpy(aprs_api, equal_sign + 1, sizeof(aprs_api));
                    aprs_api[strcspn(aprs_api, "\n")] = '\0'; // Remove the newline character
                }
            }
        }

        fclose(aprs_dat_file);
    } else {
        fprintf(stderr, "Error opening aprs.dat for reading.\n");
        return 1;
    }

    char buffer[MAX_BUFFER_SIZE] = "";

    CURL *curl = curl_easy_init();
    if (curl) {
        char api_url[MAX_BUFFER_SIZE];
        snprintf(api_url, sizeof(api_url), "https://api.aprs.fi/api/get?name=%s&what=loc&apikey=%s&format=json", aprs_call, aprs_api);
        curl_easy_setopt(curl, CURLOPT_URL, api_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            const char *latitude = ""; // Replace with JSON parsing
            const char *longitude = ""; // Replace with JSON parsing

            char *grid_square = calculateMaidenheadGridSquare(latitude, longitude);
            printf("Maidenhead Grid Square: %s\n", grid_square);
        } else {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Error initializing libcurl.\n");
        return 1;
    }

    return 0;
}
