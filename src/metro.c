/*
 * metro.c - proces startowy symulacji.
 *
 * Sprawdza parametry, uruchamia procesy ról przez fork() + exec(), czeka na ich
 * zakończenie i raportuje wynik. Tworzenie i usuwanie IPC dojdzie w kolejnym etapie.
 */
#include "common.h"
#include "errors.h"
#include "log.h"
#include "params.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Katalog z programami ról (ten sam, w którym leży metro) */
static char bin_dir[PATH_MAX];

/* Ustala katalog programów na podstawie ścieżki uruchomienia */
static void resolve_bin_dir(const char *argv0)
{
    ssize_t len = readlink("/proc/self/exe", bin_dir, sizeof(bin_dir) - 1);
    char *slash;

    if (len == -1) {
        /* brak /proc - korzystamy ze ścieżki z linii poleceń */
        snprintf(bin_dir, sizeof(bin_dir), "%s", argv0);
    } else {
        bin_dir[len] = '\0';
    }
    slash = strrchr(bin_dir, '/');
    if (slash != NULL) *slash = '\0';
    else snprintf(bin_dir, sizeof(bin_dir), ".");
}

/*
 * Uruchamia program roli jako proces potomny. args to tablica argumentów
 * zakończona NULL (bez nazwy programu). Zwraca PID dziecka.
 */
static pid_t spawn(const char *program, char *const args[])
{
    char path[PATH_MAX + 64];
    char *argv[16];
    int argc = 0;
    pid_t pid;

    snprintf(path, sizeof(path), "%s/%s", bin_dir, program);
    argv[argc++] = (char *)program;
    for (int i = 0; args != NULL && args[i] != NULL && argc < 15; i++) argv[argc++] = args[i];
    argv[argc] = NULL;

    pid = (pid_t)CHECK(fork(), "fork");
    if (pid == 0) {
        execv(path, argv);
        error_report(ERR_WARN, path);   /* exec wraca tylko przy błędzie */
        _exit(EXIT_FAILURE);
    }
    return pid;
}

int main(int argc, char *argv[])
{
    params_t params;
    char id[16];
    int children = 0;
    int failed = 0;

    error_set_role("metro");
    if (params_parse(argc, argv, &params) == -1) return EXIT_FAILURE;

    resolve_bin_dir(argv[0]);
    log_init(LOG_MAIN_FILE, "metro", COLOR_NONE);
    log_msg("start: P=%d R=%d N=%d T=%ds A=%d-%ds B=%d-%ds K=%d M=%d ziarno=%u "
            "(miejsca dla pasażerów w limicie procesów: %d)",
            params.capacity, params.bikes, params.trains, params.stop_time, params.trip_a.min,
            params.trip_a.max, params.trip_b.min, params.trip_b.max, params.cashiers,
            params.passengers, params.seed, params.passenger_slots);

    spawn("zawiadowca", NULL);
    children++;
    for (int i = 0; i < params.cashiers; i++) {
        snprintf(id, sizeof(id), "%d", i + 1);
        spawn("kasa", (char *const[]){id, NULL});
        children++;
    }
    for (int i = 0; i < params.trains; i++) {
        snprintf(id, sizeof(id), "%d", i + 1);
        spawn("maszynista", (char *const[]){id, NULL});
        children++;
    }
    spawn("generator", NULL);
    children++;

    /* Czekamy na wszystkie procesy potomne (blokująco, bez aktywnego czekania) */
    while (children > 0) {
        int status;
        pid_t pid = wait(&status);

        if (pid == -1) {
            error_report(ERR_WARN, "wait");
            break;
        }
        children--;
        if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
            failed++;
            log_msg("proces %d zakończył się błędem (status %d)", (int)pid, status);
        }
    }

    log_msg("koniec symulacji, procesy zakończone z błędem: %d", failed);
    log_close();
    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
