/*
 * errors.c - zgłaszanie i obsługa błędów (opis w errors.h).
 */
#include "errors.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static cleanup_fn_t cleanup_fn = NULL;
static const char *role_name = "metro";

void error_set_cleanup(cleanup_fn_t fn)
{
    cleanup_fn = fn;
}

void error_set_role(const char *role)
{
    if (role != NULL) role_name = role;
}

/* Kończy proces po błędzie krytycznym; sprzątanie wywoływane jest tylko raz */
static void fatal_exit(void)
{
    cleanup_fn_t fn = cleanup_fn;

    cleanup_fn = NULL;   /* błąd w trakcie sprzątania nie wywoła go ponownie */
    if (fn != NULL) fn();
    exit(EXIT_FAILURE);
}

void error_report(err_level_t level, const char *where)
{
    char label[256];
    int saved_errno = errno;

    snprintf(label, sizeof(label), "%s [%d]: %s", role_name, (int)getpid(), where);
    errno = saved_errno;
    perror(label);

    if (level == ERR_FATAL) fatal_exit();
}

void error_message(err_level_t level, const char *fmt, ...)
{
    va_list ap;

    fprintf(stderr, "%s: ", role_name);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);

    if (level == ERR_FATAL) fatal_exit();
}

long error_check(long rc, const char *where)
{
    if (rc == -1) error_report(ERR_FATAL, where);
    return rc;
}
