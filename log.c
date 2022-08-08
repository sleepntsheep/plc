#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#define va(x) \
        va_list a; \
        va_start(a, fmt); \
        x; \
        va_end(a); 

void
__stderr_log(const char *type, const char *file
        , const int line, const char *fmt
        , ...)
{
    fprintf(stderr, "%s: %s:%d: ", type, file, line);
    va(vfprintf(stderr, fmt, a));
    fprintf(stderr, "\n");
}

void
_panic(const char *file, const int line,
        const char *fmt, ...)
{
    va(__stderr_log("panic", file, line, fmt, a));
    exit(EXIT_FAILURE);
}

void
_panicerr(const char *file, const int line,
        const char *fmt, ...)
{
    va(__stderr_log("panic", file, line, fmt, a));
    perror("");
    exit(EXIT_FAILURE);
}

void
_warn(const char *file, const int line,
        const char *fmt, ...)
{
    va(__stderr_log("warn", file, line, fmt, a));
}

void
_warnerr(const char *file, const int line,
        const char *fmt, ...)
{
    va(__stderr_log("warn", file, line, fmt, a));
    perror("");
}

void
_info(const char *file, const int line,
        const char *fmt, ...)
{
    va(__stderr_log("info", file, line, fmt, a));
}

void
_infoerr(const char *file, const int line,
        const char *fmt, ...)
{
    va(__stderr_log("info", file, line, fmt, a));
    perror("");
}

