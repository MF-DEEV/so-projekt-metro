/*
 * maszynista.c - maszynista - prowadzi jeden pociąg: wpuszcza pasażerów z ważnym biletem,
 * pilnuje limitów P i R, odjeżdża co T i wraca po czasie Ti. Argument: numer pociągu.
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

    log_init(LOG_MAIN_FILE, "maszynista", COLOR_BLUE);
    log_msg("start (nr %s)", id);
    log_msg("koniec (nr %s)", id);
    log_close();
    return EXIT_SUCCESS;
}
