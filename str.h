#pragma once
#ifndef PLC_STR_H
#define PLC_STR_H

#include <stddef.h>
#include <stdio.h>
#include "vec.h"

typedef struct str str;
struct str
{
    char *b; // buffer
    size_t c, l; // capacity, length
};

struct str
*str_init();

struct str
*str_charp(char *p);

struct str *
str_push(struct str *s,
        char *n);

struct vec *
str_split_ch(struct str *s,
        char c);

void
str_free(void *s);

void
str_resize(struct str *s, size_t newsz);

void 
str_npush(struct str *s,
        char *n, size_t len);

void
str_sprintf(struct str *s,
        const char *fmt, ...);

struct str *
str_aprintf(const char *fmt, ...);

#endif
