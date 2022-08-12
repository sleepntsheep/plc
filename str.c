#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "str.h"
#include "xmalloc.h"

typedef struct str str;
struct str*
	__str_init()
{
	struct str* s = xmalloc(sizeof * s);
	s->l = 0;
	s->c = 512;
	s->b = xcalloc(s->c, 1);
	return s;
}

struct str*
	str_new(char* p)
{
	struct str* str = __str_init();
	if (p != NULL)
		str_push(str, p);
	return str;
}

struct str*
	str_push(struct str* s,
		char* n)
{
	size_t nlen = strlen(n);
	if (s->l + nlen >= s->c)
		str_resize(s, s->l + nlen + 1);
	strcpy(s->b + s->l, n);
	s->l += nlen;
	return s;
}

struct str*
	str_npush(struct str* s,
		char* n, size_t len)
{
	if (s->l + len >= s->c)
		str_resize(s, s->l + len + 1);
	strncpy(s->b + s->l, n, len);
	s->l += len;
	return s;
}

struct vec* str_split_str(struct str* haystack,
	struct str* needle)
{
	struct vec* v = vec_init();
	long off = 0;
	long linelen = 0;
	for (;;)
	{
		char* sp = haystack->b + off;
		char* ch = strstr(sp, needle->b);
		if (ch == NULL)
			break;
		linelen = ch - sp;
		struct str* line = str_npush(str_new(0), sp, linelen);
		str_push(line, ""); // add nil;
		vec_push(v, line);
		off += linelen + needle->l;
	}
	return v;
}

struct vec* str_split_ch(struct str* s,
	char c)
{
	struct vec* v = vec_init();
	long off = 0;
	long linelen = 0;
	for (;;)
	{
		char* sp = s->b + off;
		char* ch = strchr(sp, c);
		if (!ch) break;
		linelen = ch - sp;
		char* line = xmalloc(linelen + 2);
		strncpy(line, sp, linelen);
		line[linelen] = 0;
		vec_push(v, str_new(line));
		free(line);
		off += linelen + 1;
	}
	return v;
}

void str_free(struct str* s)
{
	free(s->b);
	free(s);
}

struct str*
str_resize(struct str* s, size_t newsz)
{
	if (newsz > s->c)
	{
		s->b = xrealloc(s->b, s->c = newsz);
	}
	else if (newsz < s->l)
	{
		s->l = newsz;
		s->b[s->l] = 0;
	}
	return s;
}

size_t
__sprintf_sz(const char* fmt, ...)
{
	size_t len;
	va_list args;
	va_start(args, fmt);
	len = vsnprintf(NULL, 0, fmt, args);
	va_end(args);
	return len;
}

void
str_sprintf(struct str* s,
	const char* fmt, ...)
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

struct str*
	str_aprintf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	size_t len = __sprintf_sz(fmt, args);
	va_end(args);
	struct str* s = str_new(0);
	str_resize(s, len);
	va_start(args, fmt);
	vsnprintf(s->b, len, fmt, args);
	va_end(args);
	return s;
}

