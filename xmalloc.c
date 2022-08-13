#include <stdlib.h>
#include <stdio.h>
#include "xmalloc.h"
#include "log.h"

void *check_alloc(void *ptr) {
    if (ptr == NULL)
        warn("allocation failed");
    return ptr;
}

void *xmalloc(size_t size) {
    return check_alloc(malloc(size));
}

void *xcalloc(size_t nmemb, size_t size) {
    return check_alloc(calloc(nmemb, size));
}

void *xrealloc(void *ptr, size_t size) {
    if (size == 0) 
        return warn("realloc: size 0"), NULL;
    return check_alloc(realloc(ptr, size));
}

FILE *xfopen(const char* s, const char* mode) {
    FILE* fp = fopen(s, mode);
    if (fp == NULL)
        warn("xfopen: failed opening %s with mode %s", s, mode);
    return fp;
}
