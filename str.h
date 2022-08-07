#pragma once
#ifndef PLC_STR_H
#define PLC_STR_H

#include <stddef.h>
#include "vec.h"

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
str_npush(struct str *s,
        char *n, size_t len);

#endif
