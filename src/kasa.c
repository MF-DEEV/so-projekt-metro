/*
 * kasa.c - kasa - stanowisko kasowe: sprzedaje bilety na kierunek A lub B
 * i rejestruje PID każdej osoby wchodzącej na dworzec. Argument: numer stanowiska.
 *
 * Etap 1: szkielet procesu (zapis startu i końca w logu).
 */
#include "common.h"
#include "errors.h"
#include "log.h"

#include <stdlib.h>

int main(int argc, char *argv[])
{
    const char *id = argc > 1 ? argv[1] : "?";

    log_init(LOG_MAIN_FILE, "kasa", COLOR_GREEN);
    log_msg("start (nr %s)", id);
    log_msg("koniec (nr %s)", id);
    log_close();
    return EXIT_SUCCESS;
}
