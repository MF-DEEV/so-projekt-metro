/*
 * params.h - parametry symulacji i ich walidacja.
 */
#ifndef METRO_PARAMS_H
#define METRO_PARAMS_H

/* Zakres czasu kursu w jednym kierunku (sekundy) */
typedef struct {
    int min;
    int max;
} range_t;

typedef struct {
    int capacity;        /* P - pojemność pociągu */
    int bikes;           /* R - miejsca na rowery */
    int trains;          /* N - liczba pociągów */
    int stop_time;       /* T - czas postoju na stacji [s] */
    range_t trip_a;      /* Ti dla kierunku A [s] */
    range_t trip_b;      /* Ti dla kierunku B [s] */
    int cashiers;        /* K - liczba stanowisk kasowych */
    int passengers;      /* M - łączna liczba pasażerów */
    unsigned int seed;   /* ziarno losowania */
    int passenger_slots; /* ilu pasażerów może istnieć jednocześnie (limit procesów) */
} params_t;

/* Wynik params_parse(), gdy użytkownik poprosił o pomoc (-h) */
#define PARAMS_HELP 1

/*
 * Odczytuje parametry z linii poleceń i sprawdza ich poprawność oraz limit
 * procesów użytkownika. Zwraca 0 przy sukcesie, PARAMS_HELP po wypisaniu
 * pomocy, -1 gdy dane są niepoprawne (komunikat jest już wypisany).
 */
int params_parse(int argc, char *argv[], params_t *out);

/* Wypisuje sposób użycia programu */
void params_usage(const char *prog);

#endif /* METRO_PARAMS_H */
