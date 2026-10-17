/*
 * common.h - stałe i typy wspólne dla wszystkich procesów symulacji metra.
 */
#ifndef METRO_COMMON_H
#define METRO_COMMON_H

/* Katalog i pliki logów (ścieżki względne do katalogu uruchomienia) */
#define LOG_DIR       "logs"
#define LOG_MAIN_FILE "logs/symulacja.log"
#define LOG_KASA_FILE "logs/kasa.log"

/* Prawa dostępu do plików i struktur IPC: tylko właściciel */
#define PERM_OWNER 0600

/* Kierunki jazdy pociągu */
typedef enum {
    DIR_A = 0,
    DIR_B = 1
} direction_t;

/* Liczba wejść do pociągu: 1 i 2 dla bagażu, 3 dla rowerów */
#define ENTRANCES     3
#define ENTRANCE_BIKE 2   /* indeks wejścia dla rowerów (liczony od 0) */

/* Wiek, poniżej którego dziecko wymaga opiekuna */
#define CHILD_AGE_LIMIT 8

/* Procesy stałe poza pasażerami: metro, zawiadowca, generator */
#define FIXED_PROCESSES 3

/* Zapas procesów pozostawiony dla powłoki użytkownika i narzędzi (ps, kill) */
#define PROCESS_RESERVE 8

#endif /* METRO_COMMON_H */
