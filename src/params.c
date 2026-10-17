/*
 * params.c - odczyt i walidacja parametrów symulacji (opis w params.h).
 */
#include "params.h"

#include "common.h"
#include "errors.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

/* Granice parametrów (opisane w docs/ i README) */
#define MAX_CAPACITY   1000
#define MAX_TRAINS     10
#define MAX_SECONDS    3600
#define MAX_CASHIERS   10
#define MAX_PASSENGERS 100000

/* Gdy system nie ogranicza liczby procesów, przyjmujemy rozsądny pułap */
#define NPROC_FALLBACK 1000

void params_usage(const char *prog)
{
    fprintf(stderr,
            "Użycie: %s [-P pojemność] [-R rowery] [-N pociągi] [-T postój_s]\n"
            "          [-A min-max] [-B min-max] [-K kasy] [-M pasażerowie] [-s ziarno]\n"
            "  -P  pojemność pociągu, 1-%d (domyślnie 20)\n"
            "  -R  miejsca na rowery, 0-P (domyślnie 4)\n"
            "  -N  liczba pociągów, 1-%d (domyślnie 3)\n"
            "  -T  czas postoju na stacji w sekundach, 1-%d (domyślnie 5)\n"
            "  -A  czas kursu w kierunku A w sekundach, np. 4-8\n"
            "  -B  czas kursu w kierunku B w sekundach, np. 6-12\n"
            "  -K  liczba stanowisk kasowych, 1-%d (domyślnie 2)\n"
            "  -M  łączna liczba pasażerów, 1-%d (domyślnie 300)\n"
            "  -s  ziarno losowania (domyślnie czas systemowy)\n",
            prog, MAX_CAPACITY, MAX_TRAINS, MAX_SECONDS, MAX_CASHIERS, MAX_PASSENGERS);
}

/* Zamienia tekst na liczbę całkowitą z zakresu [min, max]; -1 przy błędzie */
static int parse_int(const char *text, const char *name, int min, int max, int *out)
{
    char *end;
    long value;

    errno = 0;
    value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        error_message(ERR_WARN, "%s: \"%s\" nie jest liczbą całkowitą", name, text);
        return -1;
    }
    if (value < min || value > max) {
        error_message(ERR_WARN, "%s: wartość %ld spoza zakresu %d-%d", name, value, min, max);
        return -1;
    }
    *out = (int)value;
    return 0;
}

/* Odczytuje zakres w postaci "min-max" (np. "4-8"); -1 przy błędzie */
static int parse_range(const char *text, const char *name, range_t *out)
{
    char buf[64];
    char *dash;

    if (strlen(text) >= sizeof(buf)) {
        error_message(ERR_WARN, "%s: zbyt długi zapis zakresu", name);
        return -1;
    }
    strcpy(buf, text);
    dash = strchr(buf, '-');
    if (dash == NULL) {
        error_message(ERR_WARN, "%s: oczekiwano zakresu w postaci min-max, np. 4-8", name);
        return -1;
    }
    *dash = '\0';
    if (parse_int(buf, name, 1, MAX_SECONDS, &out->min) == -1) return -1;
    if (parse_int(dash + 1, name, 1, MAX_SECONDS, &out->max) == -1) return -1;
    if (out->min > out->max) {
        error_message(ERR_WARN, "%s: minimum %d większe od maksimum %d", name, out->min, out->max);
        return -1;
    }
    return 0;
}

/*
 * Liczy zadania (procesy i wątki) bieżącego użytkownika na podstawie /proc.
 * Limit RLIMIT_NPROC w Linuksie obejmuje także wątki. Zwraca -1, gdy /proc
 * jest niedostępny.
 */
static int count_user_tasks(void)
{
    DIR *dir;
    struct dirent *entry;
    uid_t uid = getuid();
    int total = 0;

    dir = opendir("/proc");
    if (dir == NULL) return -1;

    while ((entry = readdir(dir)) != NULL) {
        char path[300];
        char line[256];
        FILE *status;
        long owner = -1;
        int threads = 0;

        if (!isdigit((unsigned char)entry->d_name[0])) continue;
        snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
        status = fopen(path, "r");
        if (status == NULL) continue;   /* proces mógł się właśnie zakończyć */
        while (fgets(line, sizeof(line), status) != NULL) {
            if (strncmp(line, "Uid:", 4) == 0) owner = strtol(line + 4, NULL, 10);
            else if (strncmp(line, "Threads:", 8) == 0) threads = (int)strtol(line + 8, NULL, 10);
        }
        fclose(status);
        if (owner == (long)uid) total += threads > 0 ? threads : 1;
    }
    if (closedir(dir) == -1) error_report(ERR_WARN, "closedir /proc");
    return total;
}

/*
 * Wylicza, ile zadań mogą jednocześnie zająć pasażerowie, i zapisuje wynik
 * w out->passenger_slots. Pasażer to 1 zadanie, pasażer z dzieckiem - 2.
 */
static int check_process_limit(params_t *out)
{
    struct rlimit rl;
    long limit;
    int used;
    long fixed = FIXED_PROCESSES + out->trains + out->cashiers;
    long available;

    if (getrlimit(RLIMIT_NPROC, &rl) == -1) {
        error_report(ERR_WARN, "getrlimit RLIMIT_NPROC");
        return -1;
    }
    limit = (rl.rlim_cur == RLIM_INFINITY || rl.rlim_cur > INT_MAX) ? NPROC_FALLBACK : (long)rl.rlim_cur;

    used = count_user_tasks();
    if (used < 0) used = 0;   /* brak /proc (np. macOS) - liczymy tylko własne procesy */

    available = limit - used - fixed - PROCESS_RESERVE;
    if (available < 2) {
        error_message(ERR_WARN,
                      "za mało procesów: limit %ld, zajęte %d, symulacja potrzebuje %ld stałych "
                      "+ %d zapasu + co najmniej 2 dla pasażerów. Zmniejsz N lub K.",
                      limit, used, fixed, PROCESS_RESERVE);
        return -1;
    }
    out->passenger_slots = available > INT_MAX ? INT_MAX : (int)available;
    return 0;
}

int params_parse(int argc, char *argv[], params_t *out)
{
    int opt;
    int bikes_given = 0;

    *out = (params_t){
        .capacity = 20,
        .bikes = 4,
        .trains = 3,
        .stop_time = 5,
        .trip_a = {4, 8},
        .trip_b = {6, 12},
        .cashiers = 2,
        .passengers = 300,
        .seed = (unsigned int)time(NULL),
        .passenger_slots = 0,
    };

    opterr = 0;   /* komunikaty o błędnych opcjach wypisujemy sami */
    while ((opt = getopt(argc, argv, "P:R:N:T:A:B:K:M:s:h")) != -1) {
        int rc = 0;
        int seed;

        switch (opt) {
        case 'P': rc = parse_int(optarg, "-P (pojemność)", 1, MAX_CAPACITY, &out->capacity); break;
        case 'R': rc = parse_int(optarg, "-R (rowery)", 0, MAX_CAPACITY, &out->bikes); bikes_given = 1; break;
        case 'N': rc = parse_int(optarg, "-N (pociągi)", 1, MAX_TRAINS, &out->trains); break;
        case 'T': rc = parse_int(optarg, "-T (postój)", 1, MAX_SECONDS, &out->stop_time); break;
        case 'A': rc = parse_range(optarg, "-A (kurs A)", &out->trip_a); break;
        case 'B': rc = parse_range(optarg, "-B (kurs B)", &out->trip_b); break;
        case 'K': rc = parse_int(optarg, "-K (kasy)", 1, MAX_CASHIERS, &out->cashiers); break;
        case 'M': rc = parse_int(optarg, "-M (pasażerowie)", 1, MAX_PASSENGERS, &out->passengers); break;
        case 's':
            rc = parse_int(optarg, "-s (ziarno)", 0, INT_MAX, &seed);
            out->seed = (unsigned int)seed;
            break;
        case 'h':
            params_usage(argv[0]);
            return -1;
        default:
            if (optopt != 0 && strchr("PRNTABKMs", optopt) != NULL)
                error_message(ERR_WARN, "opcja -%c wymaga wartości", optopt);
            else
                error_message(ERR_WARN, "nieznana opcja -%c", optopt);
            params_usage(argv[0]);
            return -1;
        }
        if (rc == -1) return -1;
    }
    if (optind < argc) {
        error_message(ERR_WARN, "nieoczekiwany argument \"%s\"", argv[optind]);
        params_usage(argv[0]);
        return -1;
    }

    if (out->bikes > out->capacity) {
        if (bikes_given) {
            error_message(ERR_WARN, "-R (rowery): %d przekracza pojemność pociągu P = %d",
                          out->bikes, out->capacity);
            return -1;
        }
        out->bikes = out->capacity;   /* domyślne R dopasowane do małego P */
    }

    return check_process_limit(out);
}
