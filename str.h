#pragma once
#ifndef PLC_STR_H
#define PLC_STR_H

#include <stddef.h>
#include <stdio.h>


typedef struct str str;
struct str
{
	char* b; // buffer
	size_t c, l; // capacity, length
};

#define str_split(a,b) \
	_Generic((b), \
		struct str: str_split_str, \
		default: str_split_cstr \
	)(a,b)

#define str_cat(a,b) \
	_Generic((b), \
		struct str: str_cat_str, \
		default: str_cat_cstr \
	)(a,b)

struct str
str_new();

struct str
cstr(char* p);

struct str
str_cat_str(struct str s,
		struct str n);

struct str
str_cat_cstr(struct str s,
		char* n);

struct str*
str_split_str(struct str haystack,
		struct str needle);

struct str*
str_split_cstr(struct str haystack,
		char* needle);

void
str_free(struct str* s);

struct str
str_resize(struct str s, size_t newsz);

struct str
str_aprintf(const char* fmt, ...);

#endif /* PLC_STR_H */