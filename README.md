Opis działania

Program realizuje następujące zadania:

-generuje losowe dane pomiarowe (np. temperaturę),
-pakuje je do postaci binarnej,
-wysyła przez UDP do wskazanego serwera,
-wypisuje dane w czytelnej formie na ekran,
-działa w nieskończonej pętli co określony czas (cykl).

Parametry wejściowe

Program uruchamia się z trzema argumentami przekazanymi w linii poleceń:


./bot <IP_SERWERA> <PORT> <ODSTĘP_W_SEKUNDACH>

Przykład:


./bot 127.0.0.1 5000 2
Co 2 sekundy wysyła pakiet na serwer UDP pod adresem `127.0.0.1:5000`.

---

Struktura danych pakietu

Każdy pakiet zawiera następujące pola:

| Pole            | Typ        | Opis                                   |
| --------------- | ---------- | -------------------------------------- |
| timestamp       | `uint32_t` | Znacznik czasu (sekundy od epoki UNIX) |
| temperatura     | `uint16_t` | Temperatura ×10, zakres 200–1200       |
| zasilanie       | `uint8_t`  | 0 = zasilanie sieciowe, 1 = bateryjne  |
| id\_pomiaru     | `uint8_t`  | Licznik pomiarów (0–255, zawijany)     |
| suma\_kontrolna | `uint8_t`  | Suma wszystkich bajtów poza sobą samą  |


## Testowanie:

1. Kompilacja programu:

```bash
gcc main.c -o bot
```

2. Uruchomienie:

```bash
./bot 127.0.0.1 5000 2
```

3.Nasłuchiwanie pakietów:

```bash
nc -u -l -p 5000
```

---

## Wymagania systemowe

* System operacyjny: Linux lub WSL
* Kompilator: gcc
* Narzędzie netcat

## Autor:
Przemysław Tomaszewski