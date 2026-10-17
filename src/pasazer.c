/*
 * pasazer.c - pasazer - pasażer metra: kupuje bilet (VIP ma go wcześniej), wybiera
 * najkrótszą kolejkę do wejścia i wsiada do pociągu. Dziecko poniżej 8 lat to wątek
 * w procesie opiekuna.
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

    log_init(LOG_MAIN_FILE, "pasazer", COLOR_CYAN);
    log_msg("start");
    log_msg("koniec");
    log_close();
    return EXIT_SUCCESS;
}
