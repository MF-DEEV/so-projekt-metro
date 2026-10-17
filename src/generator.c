/*
 * generator.c - generator - tworzy procesy pasażerów w losowych momentach, pilnując
 * limitu procesów użytkownika.
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

    log_init(LOG_MAIN_FILE, "generator", COLOR_MAGENTA);
    log_msg("start");
    log_msg("koniec");
    log_close();
    return EXIT_SUCCESS;
}
