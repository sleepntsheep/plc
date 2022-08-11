#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vec.h"
#include "str.h"
#include "log.h"
#include "termcolor.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif /* PATH_MAX */
#define PLC_CONF_FILE_NAME "plc.conf"
#define PLC_DATA_FILE_NAME "plc.txt"

typedef struct str str;
typedef struct vec vec;

typedef struct {
    bool done;
    str *name;
} task;

static int
tasks_cmp(const void *_a, const void *_b)
{
    task *a = *((task**)_a);
    task *b = *((task**)_b);
    if (!a->done && b->done)
        return 1;
    if (a->done && !b->done)
        return -1;
    return strcmp(a->name->b, b->name->b);
}

task *
new_task(bool done, str *name)
{
    task *ret = malloc(sizeof(task *));
    ret->done = done;
    ret->name = name;
    return ret;
}

str *
getconfdir()
{
    char *dir = 0;
    if (!(dir = getenv("XDG_CONFIG_HOME")))
        dir = getenv("HOME");
    return str_new(dir);
}

str *
getconfpath()
{
    return str_push(getconfdir(), "/"PLC_CONF_FILE_NAME);
}

str *
getdatapath()
{
    return str_push(getconfdir(), "/"PLC_DATA_FILE_NAME);
}

void
do_task(task *t)
{
    t->done ^= 1;
}

void
clean_tasks(vec *v)
{
    for (int i = 0; i < v->l; i++)
        if (((task*)v->a[i])->done)
            vec_del(v, i--);
}

vec *
read_tasks()
{
    str *path = getdatapath();
    /* read file to  string */
    FILE *fp = fopen(path->b, "r");
    if (!fp)
        panicerr("fopen: failed opening file");
    str_free(path);
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *s = malloc(fsize + 1);
    fread(s, fsize, 1, fp);
    fclose(fp);
    /* split \n */
    vec *lines = str_split_ch(str_new(s), '\n');
    /* free */
    free(s);
    /* read tasks from vec str */
    vec *tasks = vec_init();
    for (size_t i = 0; i < lines->l; i++)
    {
        str *line = ((str*)lines->a[i]);
        vec_push(tasks, new_task(line->b[1] == 'x', str_new(line->b+4)));
    }
    vec_free(lines);
    return tasks;
}

void
write_tasks(vec *v)
{
    str *path = getdatapath();
    FILE *fp = fopen(path->b, "w");
    str_free(path);
    for (size_t i = 0; i < v->l; i++)
    {
        task *task = (v->a[i]);
        fprintf(fp, "[%c] %s\n",
            task->done ? 'x' : ' ',
            task->name->b
        );
    }
    fclose(fp);
}

void
show_tasks(vec *v)
{
    for (size_t i = 0; i < v->l; i++)
    {
        task *task = v->a[i];
        printf("%s", task->done ? FGGREEN : FGRED);
        printf("%ld [%c] %s\n", i, task->done?'x':' ', task->name->b);
    }
}

int 
main(int argc,
        char **argv)
{
    vec *v = read_tasks();

    for (++argv;*argv;argv++)
    {
        if (!strcmp(*argv, "add"))
        {
            if (*(++argv) == NULL)
                panic("add need argument");
            str *task_name = str_new(0);
            for (; *argv; argv++)
            {
                if (argv[1] != NULL)
                    str_push(task_name, " ");
                str_push(task_name, *argv);
            }
            vec_push(v, new_task(false, task_name));
        }
        else if (!strcmp(*argv, "do"))
        {
            if (argv[1] == NULL)
                panic("do need argument");
            else 
                do_task(vec_at(v, atoi(argv[1])));
        }
        else if (!strcmp(*argv, "clean"))
        {
            clean_tasks(v);
        }
        else if (!strcmp(*argv, "del"))
        {
            if (argv[1] == NULL)
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
