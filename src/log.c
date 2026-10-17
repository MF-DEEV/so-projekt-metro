/*
 * log.c - zapis przebiegu symulacji (opis w log.h).
 */
#include "log.h"

#include "common.h"
#include "errors.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define LOG_LINE_MAX 512

static int log_fd = -1;
static const char *log_role = "metro";
static log_color_t log_color = COLOR_NONE;
static int use_color = 0;

static const char *const ansi[] = {
    [COLOR_NONE] = "",
    [COLOR_RED] = "\033[31m",
    [COLOR_GREEN] = "\033[32m",
    [COLOR_YELLOW] = "\033[33m",
    [COLOR_BLUE] = "\033[34m",
    [COLOR_MAGENTA] = "\033[35m",
    [COLOR_CYAN] = "\033[36m",
};

void log_init(const char *path, const char *role, log_color_t color)
{
    if (role != NULL) log_role = role;
    log_color = color;
    use_color = isatty(STDOUT_FILENO);
    error_set_role(log_role);

    /* Katalog może już istnieć - to nie jest błąd */
    if (mkdir(LOG_DIR, 0700) == -1 && errno != EEXIST)
        error_report(ERR_FATAL, "mkdir " LOG_DIR);

    log_fd = (int)CHECK(open(path, O_WRONLY | O_CREAT | O_APPEND, PERM_OWNER), "open log");
}

/* Bieżący czas w formacie HH:MM:SS.mmm */
static void format_time(char *buf, size_t size)
{
    struct timespec ts;
    struct tm tm;

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        error_report(ERR_WARN, "clock_gettime");
        snprintf(buf, size, "--:--:--.---");
        return;
    }
    localtime_r(&ts.tv_sec, &tm);
    snprintf(buf, size, "%02d:%02d:%02d.%03d", tm.tm_hour % 24, tm.tm_min % 60, tm.tm_sec % 61,
             (int)(ts.tv_nsec / 1000000L) % 1000);
}

void log_msg(const char *fmt, ...)
{
    char now[32];
    char text[LOG_LINE_MAX];
    char line[LOG_LINE_MAX + 64];
    va_list ap;
    int len;

    format_time(now, sizeof(now));
    va_start(ap, fmt);
    vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);

    len = snprintf(line, sizeof(line), "[%s] [%-10s %6d] %s\n", now, log_role, (int)getpid(), text);
    if (len < 0) return;
    if ((size_t)len >= sizeof(line)) len = (int)sizeof(line) - 1;

    if (log_fd != -1 && write(log_fd, line, (size_t)len) == -1)
        error_report(ERR_WARN, "write log");

    if (use_color && log_color != COLOR_NONE)
        printf("%s%.*s\033[0m\n", ansi[log_color], len - 1, line);
    else
        fputs(line, stdout);
    fflush(stdout);
}

void log_close(void)
{
    if (log_fd != -1 && close(log_fd) == -1) error_report(ERR_WARN, "close log");
    log_fd = -1;
}
