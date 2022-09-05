#define SHEEP_DYNARRAY_IMPLEMENTATION
#include "dynarray.h"
#define SHEEP_STR_HAVE_STB_DS
#define SHEEP_STR_IMPLEMENTATION
#include "str.h"
#define SHEEP_LOG_IMPLEMENTATION
#include "log.h"
#include "arg.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif /* PATH_MAX */
#define PLC_DATA_FILE_NAME "plc.txt"

#define _FG(x) "\033[38;5;"#x"m"
#define FGRED _FG(9)
#define FGGREEN _FG(3)
#define FGBLUE _FG(4)
#define FGRST "\033[39m"

typedef struct {
	bool done;
	str  name;
} task_t;

static int
tasks_cmp(const void* _a, const void* _b)
{
	task_t a = *((task_t*)_a);
	task_t b = *((task_t*)_b);
	if (!a.done && b.done)
		return 1;
	if (a.done && !b.done)
		return -1;
	return strcmp(a.name.b, b.name.b);
}

str
getconfdir()
{
	char* dir = NULL;
	if (!(dir = getenv("XDG_CONFIG_HOME")))
		if (!(dir = getenv("HOME")))
			dir = ".";
	return cstr(dir);
}

str
getdatapath()
{
	return str_cat_cstr(str_dup(getconfdir()), "/"PLC_DATA_FILE_NAME);
}

void
do_task(task_t* t)
{
	t->done ^= 1;
}

void
clean_tasks(task_t* tasks)
{
	for (size_t i = 0; i < arrlen(tasks); i++)
		if (tasks[i].done)
			arrdel(tasks, i);
}

task_t *
read_tasks()
{
	str path = getdatapath();
	/* read file to  string */
	FILE* fp = fopen(path.b, "ab+");
	fseek(fp, 0L, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char* s = calloc(fsize + 1, 1);
	fread(s, 1, fsize, fp);
	fclose(fp);
	/* split newline
	* we opened file with ab+, so there's no \r\n and \n translation going on
	* if we don't open file as binary, SEEK_END and SEEK_SET will break 
	* on MSVC compiler
	*/
#ifdef _WIN32
	str* lines = str_split_cstr(cstr(s), "\r\n");
#else
	str* lines = str_split_cstr(cstr(s), "\n");
#endif
	/* read tasks from str */
	task_t* tasks = arrnew(task_t);
	for (size_t i = 0; i < arrlen(lines); i++)
	{
		if (lines[i].l < 4)
			continue; // empty line
		arrpush(tasks, ((task_t) {
			.done = lines[i].b[1] == 'x',
			.name = cstr(lines[i].b + 4)
		}));
	}
	return tasks;
}

void
write_tasks(task_t* tasks)
{
	str path = getdatapath();
	FILE* fp = fopen(path.b, "w");
	for (size_t i = 0; i < arrlen(tasks); i++)
	{
		fprintf(fp, "[%c] %s\n",
			tasks[i].done ? 'x' : ' ',
			tasks[i].name.b
		);
	}
	fclose(fp);
}

void
show_tasks(task_t* tasks)
{
	printf(FGBLUE);
	puts(welcome_message);
    int idxlen = snprintf(NULL, 0, "%zd", arrlen(tasks));
	for (size_t i = 0; i < arrlen(tasks); i++)
	{
		printf("%s", tasks[i].done ? FGGREEN : FGRED);
		printf("%*zd [%c] %s\n"
			, idxlen, i, tasks[i].done ? 'x' : ' '
			, tasks[i].name.b);
	}
	printf(FGRST);
}

void
sort_tasks(task_t *v)
{
	qsort(v, arrlen(v),
		sizeof(*v), tasks_cmp);
}

int
main(int argc,
	char** argv)
{
	task_t* v = read_tasks();

	ARGBEGIN
		ARGCMP("add") 
		{
			str t = str_new();
			ALLARG
			{
				str_cat_cstr(&t, CARG);
				if (argv[_i+1])
					str_cat_cstr(&t, " ");
			}
			arrpush(v, ((task_t) { false, t }));
		}
		ARGCMP("del")
		{
			ALLARG
			{
                size_t i = strtoul(CARG, 0, 10);
                arrdel(v, i);
            }
		}
		ARGCMP("do")
		{
			ALLARG
			{
                size_t i = strtoul(CARG, 0, 10);
                do_task(v+i);
            }
		}
		ARGCMP("clean")
		{
			clean_tasks(v);
		}
        ARGCMP("sort")
        {
            sort_tasks(v);
        }
	ARGEND

	show_tasks(v);
	write_tasks(v);
    arrfree(v);
	return 0;
}
