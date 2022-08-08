#ifndef PLC_LOG_H
#define PLC_LOG_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __FL__ __FILE__, __LINE__
#define panic(...) _panic(__FL__, __VA_ARGS__)
#define panicerr(...) _panicerr(__FL__, __VA_ARGS__)
#define warn(...) _warn(__FL__, __VA_ARGS__)
#define info(...) _info(__FL__, __VA_ARGS__)
#define infoerr(...) _infoerr(__FL__, __VA_ARGS__)

/*
#define dowhile0(x) \
        do { x } \
        while ( 0 )
#define err \
        fprintf(stderr, " %s\n", strerror(errno))
#define panic(...)     __stderr_log("panic", __FL__, __VA_ARGS__), exit(1);
#define warn(...)      __stderr_log("warn", __FL__, __VA_ARGS__)
#define info(...)      __stderr_log("info", __FL__, __VA_ARGS__)

#define panicerr(...)   do { \
                            __stderr_log("panic", __FL__, __VA_ARGS__); \
                            err; \
                            exit(1); \
                        } while (0);
#define warnerr(...)    do { \
                            __stderr_log("warn", __FL__, __VA_ARGS__); \
                            err; \
                        } while (0);
#define infoerr(...)    do { \
                            __stderr_log("info", __FL__, __VA_ARGS__); \
                            err; \
                        } while(0);
                        */

void
__stderr_log(const char *type, const char *file,
        const int line, const char *fmt, ...);

void
_panic(const char *file, const int line,
        const char *fmt);

void
_panicerr(const char *file, const int line,
        const char *fmt);

void
_warn(const char *file, const int line,
        const char *fmt);

void
_warnerr(const char *file, const int line,
        const char *fmt);

void
_info(const char *file, const int line,
        const char *fmt);

void
_infoerr(const char *file, const int line,
        const char *fmt);

#endif

