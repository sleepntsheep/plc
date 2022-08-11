#pragma once
#ifndef PLC_VEC_H
#define PLC_VEC_H

#include <stddef.h>
#define VEC_INIT_SIZE 64

struct vec
{
    void **a; /* array */
    size_t l, c; /* length, capacity */
};

struct vec *
vec_init();

struct vec *
vec_push(struct vec *v, void *p);

void *
vec_pop(struct vec *v);

void
vec_free(struct vec *v);

struct vec *
vec_del(struct vec *v, size_t i);

void
vec_sort(struct vec *v,
        int (*compar)(const void*, const void*));

void *
vec_at(struct vec *v, size_t i);

#endif
