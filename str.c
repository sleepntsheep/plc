#include <stdlib.h>
#include <string.h>
#include "str.h"

struct str *
str_init()
{
    struct str *s = malloc(sizeof(struct str*));
    s->l = 0;
    s->c = 512;
    s->b = calloc(s->c, 1);
    return s;
}

struct str *
str_charp(char *p)
{
    struct str *str = str_init();
    str_push(str, p);
    return str;
}

void
_str_extend(struct str *s)
{
    s->b = realloc(s->b, s->c *= 2);
}

struct str *
str_push(struct str *s,
        char *n)
{
    size_t nlen = strlen(n);
    if (s->l + nlen >= s->c)
        while (s->l + nlen >= s->c)
            _str_extend(s);
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
            _str_extend(s);
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
        linelen = strchr(sp, '\n') - sp;
        if (linelen <= 0) /* NULL  returned by strchr (no more \n) */
            break;
        char *line = malloc(linelen+2);
        strncpy(line, sp, linelen);
        line[linelen] = 0;
        vec_push(v, str_charp(line));
        free(line);
        off += linelen + 1;
    }
    return v;
}

void
str_free(struct str *s)
{
    free(s->b);
    free(s);
}
