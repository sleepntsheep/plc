#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "str.h"

typedef struct str str;
struct str *
__str_init()
{
    struct str *s = malloc(sizeof(struct str*));
    s->l = 0;
    s->c = 512;
    s->b = calloc(s->c, 1);
    return s;
}

struct str *
str_new(char *p)
{
    struct str *str = __str_init();
    if (p != NULL)
        str_push(str, p);
    return str;
}

struct str *
str_push(struct str *s,
        char *n)
{
    size_t nlen = strlen(n);
    if (s->l + nlen >= s->c)
        while (s->l + nlen >= s->c)
            str_resize(s, s->c * 2);
    strcpy(s->b+s->l, n);
    s->l += nlen;
    return s;
}

void
str_npush(struct str *s,
        char *n, size_t len)
{
    if (s->l + len >= s->c)
        while (s->l + len >= s->c)
            str_resize(s, s->c * 2);
    strncpy(s->b+s->l, n, len);
    s->l += len;
}

struct vec *
str_split_ch(struct str *s,
        char c)
{
    struct vec *v = vec_init();
    long off = 0;
    long linelen = 0;
    for (;;) 
    {
        char *sp = s->b + off;
        linelen = strchr(sp, c) - sp;
        if (linelen <= 0) /* NULL  returned by strchr (no more \n) */
            break;
        char *line = malloc(linelen+2);
        strncpy(line, sp, linelen);
        line[linelen] = 0;
        vec_push(v, str_new(line));
        free(line);
        off += linelen + 1;
    }
    return v;
}

void
str_free(struct str *s)
{
    free(((str*)s)->b);
    free(s);
}

void
str_resize(struct str *s, size_t newsz)
{
    if (newsz > s->c)
    {
        s->b = realloc(s->b, s->c = newsz);
    }
    else
    {
        if ((s->c = newsz) < s->l)
        {
            s->l = s->c;
            s->b[s->l] = 0;
        }
    }
}

size_t
__sprintf_sz(const char *fmt, ...)
{
    size_t len;
    va_list args;
    va_start(args, fmt);
    len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    return len;
}

void
str_sprintf(struct str *s,
        const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t len = __sprintf_sz(fmt, args);
    va_end(args);
    if (s->c < len)
        str_resize(s, len);
    va_start(args, fmt);
    vsnprintf(s->b, len, fmt, args);
    va_end(args);
}

struct str *
str_aprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t len = __sprintf_sz(fmt, args);
    va_end(args);
    struct str *s = str_new(0);
    str_resize(s, len);
    va_start(args, fmt);
    vsnprintf(s->b, len, fmt, args);
    va_end(args);
    return s;
}

