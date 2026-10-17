/*
 * log.h - zapis przebiegu symulacji do pliku tekstowego i na terminal.
 *
 * Każdy wpis to jedna linia "[czas] [rola PID] treść" zapisana jednym wywołaniem
 * write() do pliku otwartego z O_APPEND, więc wpisy z wielu procesów się nie mieszają.
 */
#ifndef METRO_LOG_H
#define METRO_LOG_H

/* Kolory ról na terminalu (sekwencje ANSI) */
typedef enum {
    COLOR_NONE,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN
} log_color_t;

/*
 * Otwiera plik logu (tworzy katalog logs/ w razie potrzeby) i ustawia rolę
 * procesu. Rola trafia też do komunikatów o błędach.
 */
void log_init(const char *path, const char *role, log_color_t color);

/* Zapisuje wpis do pliku i wypisuje go na terminal */
void log_msg(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/* Zamyka plik logu */
void log_close(void);

#endif /* METRO_LOG_H */
