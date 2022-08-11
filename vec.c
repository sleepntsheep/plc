#include "vec.h"
#include "log.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

struct vec *
vec_init()
{
    struct vec *v = malloc(sizeof(struct vec));
    v->l = 0;
    v->c = VEC_INIT_SIZE;
    v->a = calloc(v->c, sizeof(void*));
    return v;
}

size_t
check_index(struct vec *v, size_t i)
{
    if (i < 0 || i > v->l)
        warn("vec: index out of bound");
    return i;
}

void
_vec_extend(struct vec *v)
{
    v->a = realloc(v->a, v->c *= 2);
}

struct vec *
vec_push(struct vec *v, void *p)
{
    if (v->l >= v->c)
        _vec_extend(v);
    v->a[v->l++] = p;
    return v;
}

void *
vec_pop(struct vec *v)
{
    return v->a[v->l--];
}

void
vec_free(struct vec *v)
{
    for (size_t i = 0; i < v->l; i++) 
    {
        free(v->a[i]);
    }
    free(v->a);
    free(v);
}

struct vec *
vec_del(struct vec *v, size_t i)
{
    check_index(v,i);
    free(v->a[i]);
    for (; i < v->l - 1; i++)
        v->a[i] = v->a[i+1];
    v->l--;
    return v;
}

void *
vec_at(struct vec *v, size_t i)
{
    return v->a[check_index(v, i)];
}
