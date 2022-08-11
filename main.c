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

#define SVEC(v,i) ((str*)v->a[i])

task *
new_task(bool done, str *name)
{
    task *ret = malloc(sizeof(task *));
    ret->done = done;
    ret->name = name;
    return ret;
}

char *
getconfdir()
{
    char *dir = 0;
    if (!(dir = getenv("XDG_CONFIG_HOME")))
        dir = getenv("HOME");
    return dir;
}

char *
getconfpath()
{
    char *path = malloc(PATH_MAX);
    char *dir = getconfdir();
    size_t dirlen = strlen(dir);
    strcpy(path, dir);
    strcpy(path+dirlen, "/"PLC_CONF_FILE_NAME);
    return path;
}

char *
getdatapath()
{
    char *path = malloc(PATH_MAX);
    char *dir = getconfdir();
    size_t dirlen = strlen(dir);
    strcpy(path, dir);
    strcpy(path+dirlen, "/"PLC_DATA_FILE_NAME);
    return path;
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
    char *p = getdatapath();
    /* read file to  string */
    FILE *fp = fopen(p, "r");
    if (!fp)
        panicerr("fopen: failed opening file");
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *s = malloc(fsize + 1);
    fread(s, fsize, 1, fp);
    fclose(fp);
    /* split \n */
    vec *lines = str_split_ch(str_charp(s), '\n');
    /* free */
    free(s);
    free(p);
    /* read tasks from vec str */
    vec *tasks = vec_init();
    for (size_t i = 0; i < lines->l; i++)
    {
        str *line = ((str*)lines->a[i]);
        vec_push(tasks,
            new_task(line->b[1] == 'x',
                 str_charp(line->b+4)
            )
        );
    }
    vec_free(lines);
    return tasks;
}

void
write_tasks(vec *v)
{
    char *path = getdatapath();
    FILE *fp = fopen(path, "w");
    free(path);
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
            str *task_name = str_init();
            for (; *argv; argv++)
            {
                if (argv[1] != NULL)
                    str_push(task_name, " ");
                str_push(task_name, *argv);
            }
            vec_push(v, new_task(false, task_name);
        }
        else if (!strcmp(*argv, "do"))
            if (argv[1] == NULL)
                panic("do need argument");
            else 
                do_task(vec_at(v, atoi(argv[1])));
        else if (!strcmp(*argv, "clean"))
            clean_tasks(v);
        else if (!strcmp(*argv, "del"))
            if (argv[1] == NULL)
                panic("del need argument");
            else
                vec_del(v, atoi(argv[1]);
    }

    show_tasks(v);
    write_tasks(v);
    vec_free(v);
    return 0;
}
