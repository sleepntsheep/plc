#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vec.h"
#include "str.h"
#include "log.h"
#include "termcolor.h"
#include "xmalloc.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif /* PATH_MAX */
#define PLC_DATA_FILE_NAME "plc.txt"

typedef struct str str;
typedef struct vec vec;

typedef struct {
	bool done;
	str* name;
} task;

static int
tasks_cmp(const void* _a, const void* _b)
{
	task* a = *((task**)_a);
	task* b = *((task**)_b);
	if (!a->done && b->done)
		return 1;
	if (a->done && !b->done)
		return -1;
	return strcmp(a->name->b, b->name->b);
}

task*
new_task(bool done, str* name)
{
	task* ret = xmalloc(sizeof(*ret));
	ret->done = done;
	ret->name = name;
	return ret;
}

str*
getconfdir()
{
	char* dir = 0;
	if (!(dir = getenv("XDG_CONFIG_HOME")))
		if (!(dir = getenv("HOME")))
			dir = ".";
	return str_new(dir);
}

str*
getdatapath()
{
	return str_push(getconfdir(), "/"PLC_DATA_FILE_NAME);
}

void
do_task(task* t)
{
	t->done ^= 1;
}

void
clean_tasks(vec* v)
{
	for (int i = 0; i < v->l; i++)
		if (((task*)v->a[i])->done)
			vec_del(v, i--);
}

vec*
read_tasks()
{
	/* read file to  string */
	FILE* fp = xfopen(getdatapath()->b, "ab+");
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
	vec* lines = str_split_str(str_new(s), str_new(strdup("\r\n")));
#else
	vec* lines = str_split_ch(str_new(s), '\n');
#endif
	/* read tasks from vec str */
	vec* tasks = vec_init();
	for (size_t i = 0; i < lines->l; i++)
	{
		str* line = ((str*)lines->a[i]);
		if (line->l < 4)
			continue; // empty line
		vec_push(tasks, new_task(line->b[1] == 'x', str_new(line->b + 4)));
	}
	vec_free(lines);
	return tasks;
}

void
write_tasks(vec* v)
{
	str* path = getdatapath();
	FILE* fp = xfopen(path->b, "w");
	for (size_t i = 0; i < v->l; i++)
	{
		task* task = v->a[i];
		fprintf(fp, "[%c] %s\n",
			task->done ? 'x' : ' ',
			task->name->b
		);
	}
	str_free(path);
	fclose(fp);
}

void
show_tasks(vec* v)
{
	printf(FGBLUE);
	puts("Hello!! Here is your tasks");
	for (size_t i = 0; i < v->l; i++)
	{
		task* task = v->a[i];
		printf("%s", task->done ? FGGREEN : FGRED);
		printf("%zd [%c] %s\n", i, task->done ? 'x' : ' ', task->name->b);
	}
	printf(FGRST);
}

int
main(int argc,
	char** argv)
{
	vec* v = read_tasks();

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "add"))
		{
			if (argv[++i] == NULL)
				panic("add need argument");
			str* task_name = str_new(0);
			for (; i < argc; i++)
			{
				str_push(task_name, argv[i]);
				if (i < argc - 1)
					str_push(task_name, " ");
			}
			vec_push(v, new_task(false, task_name));
		}
		else if (!strcmp(argv[i], "do"))
		{
			if (argv[++i] == NULL)
				panic("do need argument");
			else
				do_task(vec_at(v, atoi(argv[1])));
		}
		else if (!strcmp(argv[i], "clean"))
		{
			clean_tasks(v);
		}
		else if (!strcmp(argv[i], "del"))
		{
			if (argv[++i] == NULL)
				panic("del need argument");
			else
				vec_del(v, atoi(argv[1]));
		}
	}

	vec_sort(v, tasks_cmp);
	show_tasks(v);
	write_tasks(v);
	vec_free(v);
	return 0;
}
