/*
 * errors.h - zgłaszanie i obsługa błędów.
 *
 * Każde wywołanie systemowe jest sprawdzane. Błąd zgłaszany jest przez perror()
 * z opisem miejsca, a błąd krytyczny dodatkowo uruchamia zarejestrowaną funkcję
 * sprzątającą (np. usunięcie IPC) i kończy proces.
 */
#ifndef METRO_ERRORS_H
#define METRO_ERRORS_H

#include <stdbool.h>

/* Rodzaj błędu decyduje o dalszym działaniu procesu */
typedef enum {
    ERR_WARN,   /* wypisz i kontynuuj */
    ERR_FATAL   /* wypisz, posprzątaj i zakończ proces */
} err_level_t;

/* Funkcja sprzątająca wywoływana przed zakończeniem procesu z powodu błędu */
typedef void (*cleanup_fn_t)(void);

/* Rejestruje funkcję sprzątającą (NULL wyłącza sprzątanie) */
void error_set_cleanup(cleanup_fn_t fn);

/*
 * Zgłasza błąd wywołania systemowego: wypisuje "<rola>: <gdzie>: <opis errno>".
 * Dla ERR_FATAL sprząta i kończy proces z kodem EXIT_FAILURE.
 */
void error_report(err_level_t level, const char *where);

/* Zgłasza błąd logiczny (bez errno), np. niepoprawne dane od użytkownika */
void error_message(err_level_t level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

/* Ustawia nazwę roli dołączaną do komunikatów (np. "kasa") */
void error_set_role(const char *role);

/*
 * Sprawdza wynik wywołania systemowego zwracającego -1 przy błędzie
 * (błąd jest krytyczny). Zwraca wynik bez zmian, np.:
 *     int fd = (int)CHECK(open(path, O_RDONLY), "open");
 */
long error_check(long rc, const char *where);
#define CHECK(expr, where) error_check((long)(expr), (where))

#endif /* METRO_ERRORS_H */
