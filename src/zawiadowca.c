/*
 * zawiadowca.c - zawiadowca - ogłasza kierunek pociągu i obsługuje polecenia
 * SIGUSR1 (odjazd niepełnego pociągu) oraz SIGUSR2 (zamknięcie dworca).
 *
 * Etap 1: szkielet procesu (zapis startu i końca w logu).
 */
#include "common.h"
#include "errors.h"
#include "log.h"

#include <stdlib.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    log_init(LOG_MAIN_FILE, "zawiadowca", COLOR_YELLOW);
    log_msg("start");
    log_msg("koniec");
    log_close();
    return EXIT_SUCCESS;
}
