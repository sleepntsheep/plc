#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "log.h"
#include "termcolor.h"
#include "xmalloc.h"
#include "arg.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif /* PATH_MAX */
#define PLC_DATA_FILE_NAME "plc.txt"

typedef struct str str;

typedef struct {
	bool done;
	str  name;
} task;

static int
tasks_cmp(const void* _a, const void* _b)
{
	task a = *((task*)_a);
	task b = *((task*)_b);
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
	return str_cat(str_dup(getconfdir()), "/"PLC_DATA_FILE_NAME);
}

void
do_task(task* t)
{
	t->done ^= 1;
}

void
clean_tasks(task* tasks)
{
	for (int i = 0; i < arrlen(tasks); i++)
		if (tasks[i].done)
			arrdel(tasks, i);
}

task*
read_tasks()
{
	str path = getdatapath();
	/* read file to  string */
	FILE* fp = xfopen(path.b, "ab+");
	fseek(fp, 0L, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char* s = xcalloc(fsize + 1, 1);
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
	str* lines = str_split(cstr(s), "\n");
#endif
	/* read tasks from str */
	task* tasks = NULL;
	for (size_t i = 0; i < arrlen(lines); i++)
	{
		if (lines[i].l < 4)
			continue; // empty line
		arrput(tasks, ((task) {
			.done = lines[i].b[1] == 'x',
			.name = cstr(lines[i].b + 4)
		}));
	}
	return tasks;
}

void
write_tasks(task* tasks)
{
	str path = getdatapath();
	FILE* fp = xfopen(path.b, "w");
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
show_tasks(task* tasks)
{
	printf(FGBLUE);
	puts("Hello!! Here is your tasks");
	for (size_t i = 0; i < arrlen(tasks); i++)
	{
		printf("%s", tasks[i].done ? FGGREEN : FGRED);
		printf("%zd [%c] %s\n"
			, i, tasks[i].done ? 'x' : ' '
			, tasks[i].name.b);
	}
	printf(FGRST);
}

int
main(int argc,
	char** argv)
{
	task* v = read_tasks();

	ARGBEGIN
		ARGCMP("add") 
		{
			str t = str_new();
			ALLARG
			{
				str_cat(&t, argv[_i]);
				if (argv[_i+1])
					str_cat(&t, " ");
			}
			arrput(v, ((task) { false, t }));
		}
		ARGCMP("del")
		{
			size_t i = strtoul(NARG, 0, 10);
			arrdel(v, i);
		}
		ARGCMP("do")
		{
			do_task(v+strtoul(NARG, 0, 10));
		}
		ARGCMP("clean")
		{
			clean_tasks(v);
		}
	ARGEND

	qsort(v, arrlen(v),
		sizeof(*v), tasks_cmp);
	show_tasks(v);
	write_tasks(v);
	return 0;
}
