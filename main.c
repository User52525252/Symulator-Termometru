#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

// Wczytywanie parametrów

struct Konfiguracja {
    char *ip;
    int port;
    int cycle;
};

int wczytaj_parametry(int argc, char *argv[], struct Konfiguracja *config) {
    if (argc < 4) {
        printf("Użycie: %s <IP> <port> <cykl>\n", argv[0]);
        return 0;
    }

    int port = atoi(argv[2]);
    int cycle = atoi(argv[3]);

    if (port < 1 || port > 65535) {
        printf("Błąd: port musi być w zakresie 1–65535\n");
        return 0;
    }

    if (cycle <= 0) {
        printf("Błąd: cykl musi być dodatnią liczbą\n");
        return 0;
    }

    config->ip = argv[1];
    config->port = port;
    config->cycle = cycle;
    return 1;
}

// Struktura pakietu

#define ZASILANIE_SIECIOWE 0
#define ZASILANIE_BATERYJNE 1

#pragma pack(push, 1)
struct Pakiet {
    uint32_t timestamp;
    uint16_t temperatura;
    uint8_t zasilanie;
    uint8_t id_pomiaru;
    uint8_t suma_kontrolna;
};
#pragma pack(pop)

// Generowanie pakietu

uint8_t oblicz_sume_kontrolna(struct Pakiet *p) {
    uint8_t *dane = (uint8_t *)p;
    uint8_t suma = 0;
    for (int i = 0; i < sizeof(struct Pakiet) - 1; i++) {
        suma += dane[i];
    }
    return suma;
}

struct Pakiet generuj_pakiet(uint8_t id) {
    struct Pakiet p;
    p.timestamp = (uint32_t)time(NULL);
    p.temperatura = 200 + rand() % (1201 - 200); // 200–1200 = 20.0–120.0°C
    p.zasilanie = rand() % 2;
    p.id_pomiaru = id;
    p.suma_kontrolna = 0;
    p.suma_kontrolna = oblicz_sume_kontrolna(&p);
    return p;
}

// Wysyłanie pakietu przez UDP

int wyslij_pakiet(struct Pakiet *p, struct Konfiguracja *cfg, int sockfd, struct sockaddr_in *addr) {
    ssize_t wyslane = sendto(sockfd, p, sizeof(struct Pakiet), 0,
                             (struct sockaddr *)addr, sizeof(*addr));
    if (wyslane < 0) {
        perror("Błąd przy wysyłaniu pakietu");
        return 0;
    }
    return 1;
}

// logowanie pakietu

void wypisz_pakiet(struct Pakiet *p) {
    time_t ts = (time_t)p->timestamp;
    char *czas_str = ctime(&ts);
    if (czas_str != NULL) {
        czas_str[strcspn(czas_str, "\n")] = '\0';
    }

    printf("Wysłano pakiet:\n");
    printf("  Czas pomiaru     : %s\n", czas_str ? czas_str : "(nieznany czas)");
    printf("  Temperatura      : %.1f°C\n", p->temperatura / 10.0);
    printf("  Zasilanie        : %s\n", p->zasilanie == 0 ? "sieciowe" : "bateryjne");
    printf("  ID pomiaru       : %u\n", p->id_pomiaru);
    printf("  Suma kontrolna   : %u\n", p->suma_kontrolna);
    printf("-----------------------------\n");
}

// Główna pętla

int main(int argc, char *argv[]) {
    struct Konfiguracja config;

    if (!wczytaj_parametry(argc, argv, &config)) {
        return 1;
    }

    srand(time(NULL)); // inicjalizacja RNG

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Błąd tworzenia gniazda");
        return 1;
    }

    struct sockaddr_in serwer;
    memset(&serwer, 0, sizeof(serwer));
    serwer.sin_family = AF_INET;
    serwer.sin_port = htons(config.port);
    inet_pton(AF_INET, config.ip, &serwer.sin_addr);

    uint8_t licznik = 1;

    while (1) {
        struct Pakiet p = generuj_pakiet(licznik);

        if (!wyslij_pakiet(&p, &config, sockfd, &serwer)) {
            break;
        }

        wypisz_pakiet(&p);

        licznik++;
        sleep(config.cycle);
    }

    close(sockfd);
    return 0;
}
