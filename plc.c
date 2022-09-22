#define SHEEP_DYNARRAY_IMPLEMENTATION
#include "dynarray.h"
#define SHEEP_STR_IMPLEMENTATION
#include "str.h"
#define SHEEP_LOG_IMPLEMENTATION
#include "log.h"
#define SHEEP_SJSON_IMPLEMENTATION
#include "sjson.h"
#include "arg.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif /* PATH_MAX */

#define PLC_CONF_FILE_NAME "plc.dat"

#define _FG(x) (color ? "\033[38;5;"#x"m" : "")
#define FGRED _FG(9)
#define FGGREEN _FG(3)
#define FGBLUE _FG(4)
#define FGRST "\033[39m"

typedef struct {
	bool done;
	str  name;
} task_t;

char *welcome_message;
bool check_mark;
bool color;
char confpath[PATH_MAX];
task_t *tasks;

static int tasks_cmp(const void* a, const void* b) {
	task_t a1 = *((task_t*)a);
	task_t b1 = *((task_t*)b);
	if (!a1.done && b1.done)
		return 1;
	if (a1.done && !b1.done)
		return -1;
	return strcmp(a1.name.b, b1.name.b);
}

char *confdir() {
	char* dir = NULL;
	if (!(dir = getenv("XDG_CONFIG_HOME")))
		if (!(dir = getenv("HOME")))
			dir = ".";
	return dir;
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
    if (color)
        printf(FGBLUE);
	if (welcome_message)
        puts(welcome_message);
    int idxlen = snprintf(NULL, 0, "%zd", arrlen(tasks));
	for (size_t i = 0; i < arrlen(tasks); i++) {
        if (color)
            printf("%s", tasks[i].done ? FGGREEN : FGRED);
        printf("%*zd ", idxlen, i);
        if (check_mark)
            printf("[%c] ", tasks[i].done ? 'x' : ' ');
        printf("%s\n", tasks[i].name.b);
	}
    if (color)
        printf(FGRST);
}

sjson *read_config() {
    str content = read_file(str_from_c(confpath));
    sjson *json = sjson_serialize(content.b, content.l);
    free(content.b);
    if (json == NULL || json->type == SJSON_INVALID)
        json = sjson_new(SJSON_OBJECT);
    return json;
}

int main(int argc, char** argv) {
    strcpy(confpath, confdir());
    strcat(confpath, "/"PLC_CONF_FILE_NAME);

    sjson *json = read_config();
    //sjson_register_logger(printf); 
    tasks = arrnew(task_t);

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
        ARGCMP("set")
        {
            char *key = NARG;
            char *value = NARG;
            sjson *setting_json = sjson_object_get(json, "settings");
            if (setting_json == NULL) {
                setting_json = sjson_new(SJSON_OBJECT);
                sjson_object_set(json, "settings", setting_json);
            }
            sjson *property = sjson_serialize(value, strlen(value));
            sjson_object_set(setting_json, key, property);
        }
	ARGEND

    {
        sjson *setting_json, *data_json;
        setting_json = sjson_object_get(json, "settings");
        if (setting_json != NULL &&
            setting_json->type == SJSON_OBJECT) {
            sjson *welcome_message_json;
            sjson *do_check_mark_json;
            sjson *do_color_json;
            welcome_message_json = sjson_object_get(setting_json, "welcome_message");
            do_check_mark_json = sjson_object_get(setting_json, "check_mark");
            do_color_json = sjson_object_get(setting_json, "color");
            if (welcome_message_json != NULL &&
                welcome_message_json->type == SJSON_STRING) {
                welcome_message = welcome_message_json->v.str;
            }
            if (do_check_mark_json != NULL) {
                check_mark = do_check_mark_json->type == SJSON_TRUE;
            }
            if (do_color_json != NULL) {
                color = do_color_json->type == SJSON_TRUE;
            }
        }
        data_json = sjson_object_get(json, "tasks");
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
                    .name = str_from_c(task_name->v.str)
                };
                arrpush(tasks, new_task);
            }
        }
    }

	show_tasks();

    qsort(tasks, arrlen(tasks), sizeof(*tasks), tasks_cmp);

    { 
        sjson *data_json = sjson_new(SJSON_ARRAY);
        for (size_t i = 0; i < arrlen(tasks); i++) {
            sjson *task_json = sjson_new(SJSON_OBJECT);
            sjson *done_json = sjson_new(tasks[i].done ? 
                    SJSON_TRUE : SJSON_FALSE);
            sjson *name_json = sjson_new(SJSON_STRING);
            name_json->v.str = tasks[i].name.b;
            sjson_object_set(task_json, "done", done_json);
            sjson_object_set(task_json, "name", name_json);
            sjson_array_push(data_json, task_json);
        }
        sjson_object_set(json, "tasks", data_json);
        sjsonbuf buf = sjson_deserialize(json);
        FILE *fp = fopen(confpath, "w");
        fwrite(buf.buf, 1, buf.len, fp);
        free(buf.buf);
        fclose(fp);
    }

    sjson_free(json);
    arrfree(tasks);
	return 0;
}
