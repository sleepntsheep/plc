#define SHEEP_DYNARRAY_IMPLEMENTATION
#include <sheeplib/dynarray.h>
#define SHEEP_STR_IMPLEMENTATION
#include <sheeplib/str.h>
#define SHEEP_LOG_IMPLEMENTATION
#include <sheeplib/log.h>
#define SHEEP_SJSON_IMPLEMENTATION
#include <sheeplib/sjson.h>
#include "arg.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif /* PATH_MAX */

#define PLC_CONF_FILE_NAME "plc.json"
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

char *welcome_message;
char confpath[PATH_MAX];
task_t *tasks;

static int tasks_cmp(const void* _a, const void* _b) {
	task_t a = *((task_t*)_a);
	task_t b = *((task_t*)_b);
	if (!a.done && b.done)
		return 1;
	if (a.done && !b.done)
		return -1;
	return strcmp(a.name.b, b.name.b);
}

char *confdir() {
	char* dir = NULL;
	if (!(dir = getenv("XDG_CONFIG_HOME")))
		if (!(dir = getenv("HOME")))
			dir = ".";
	return (dir);
}

void do_task(task_t* t) {
	t->done ^= 1;
}

str read_file(str path) {
    FILE* fp = fopen(path.b, "ab+");
    fseek(fp, 0L, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    char* s = calloc(fsize + 1, 1);
    fread(s, 1, fsize, fp);
    fclose(fp);
    return str_from_c(s);
}

void show_tasks() {
	printf(FGBLUE);
	if (welcome_message != NULL)
        puts(welcome_message);
    int idxlen = snprintf(NULL, 0, "%zd", arrlen(tasks));
	for (size_t i = 0; i < arrlen(tasks); i++) {
		printf("%s", tasks[i].done ? FGGREEN : FGRED);
		printf("%*zd [%c] %s\n"
			, idxlen, i, tasks[i].done ? 'x' : ' '
			, tasks[i].name.b);
	}
	printf(FGRST);
}

sjson *read_config() {
    str content = read_file(str_from_c(confpath));
    sjsontokarr *toks = sjson_lex(content.b);
    sjson *json = sjson_parse(toks);
    if (json->type == SJSON_INVALID)
        json = sjson_new(SJSON_OBJECT);
    return json;
}

int main(int argc, char** argv) {
    strcpy(confpath, confdir());
    strcat(confpath, "/"PLC_CONF_FILE_NAME);

    sjson *json = read_config();
    //sjson_register_logger(printf); 
    tasks = arrnew(task_t);

    {
        sjson *welcome_message_json, *data_json;
        welcome_message_json = sjson_object_get(json, "welcome_message");
        if (welcome_message_json != NULL &&
            welcome_message_json->type == SJSON_STRING) {
            welcome_message = welcome_message_json->stringvalue;
        }
        data_json = sjson_object_get(json, "data");
        if (data_json != NULL &&
            data_json->type == SJSON_ARRAY) {
            sjson_foreach(data_json, task_json) {
                sjson *task_done, *task_name;
                if (task_json->type != SJSON_OBJECT)
                    continue;
                task_done = sjson_object_get(task_json, "done");
                task_name = sjson_object_get(task_json, "name");
                task_t new_task = {
                    .done = (task_done->type == SJSON_TRUE),
                    .name = str_from_c(task_name->stringvalue)
                };
                arrpush(tasks, new_task);
            }
        }
    }


	ARGBEGIN
		ARGCMP("add") 
		{
			str t = str_new();
			ALLARG
			{
				str_catc(&t, CARG);
				if (argv[_i+1])
					str_catc(&t, " ");
			}
			arrpush(tasks, ((task_t) { false, t }));
		}
		ARGCMP("del")
		{
            int deled = 0;
			ALLARG
			{
                int i = strtoul(CARG, 0, 10);
                arrdel(tasks, i - deled);
                deled++;
            }
		}
		ARGCMP("do")
		{
			ALLARG
			{
                size_t i = strtoul(CARG, 0, 10);
                do_task(tasks+i);
            }
		}
		ARGCMP("clean")
		{
            for (size_t i = 0; i < arrlen(tasks); i++)
                if (tasks[i].done)
                    arrdel(tasks, i);
		}
        ARGCMP("sort")
        {
            qsort(tasks, arrlen(tasks),
                sizeof(*tasks), tasks_cmp);
        }
	ARGEND


	show_tasks();

    { 
        sjson *data_json = sjson_new(SJSON_ARRAY);
        for (size_t i = 0; i < arrlen(tasks); i++) {
            sjson *task_json = sjson_new(SJSON_OBJECT);
            sjson *done_json = sjson_new(tasks[i].done ? 
                    SJSON_TRUE : SJSON_FALSE);
            sjson *name_json = sjson_new(SJSON_STRING);
            name_json->stringvalue = tasks[i].name.b;
            sjson_object_set(task_json, "done", done_json);
            sjson_object_set(task_json, "name", name_json);
            sjson_array_push(data_json, task_json);
        }
        sjson_object_set(json, "data", data_json);
        FILE *fp = fopen(confpath, "w");
        //sjson_debug_print(json, 1);
        sjson_deserialize(fp, json);
        fclose(fp);
    }

    arrfree(tasks);
	return 0;
}
