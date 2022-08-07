#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vec.h"
#include "str.h"
#include "log.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif /* PATH_MAX */
#define PLC_CONF_FILE_NAME "plc.conf"
#define PLC_DATA_FILE_NAME "plc.txt"

typedef struct str str;
typedef struct vec vec;

#define SVEC(v,i) ((str*)v->a[i])

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
add_task(vec *v, char *s)
{
    vec_push(v, str_push(str_charp("[ ] "), s));
}

void
do_task(vec *v, size_t i)
{
    char *p = SVEC(v,i)->b+1;
    if (*p == 'x') *p = ' ';
    else *p = 'x';
}

extern bool 
done_task(vec *v, size_t i);

inline
bool
done_task(vec *v, size_t i)
{
    return ((str*)v->a[i])->b[1] == 'x';
}

void
del_task(vec *v, size_t i)
{
    vec_del(v, i);
}

void
clean_task(vec *v)
{
    for (int i = 0; i < v->l; i++)
    {
        if (done_task(v,i))
        {
            del_task(v,i);
            i--;
        }
    }
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
    vec *v = str_split_ch(str_charp(s), '\n');
    free(s);
    free(p);
    return v;
}

void
write_tasks(vec *v)
{
    char *p = getdatapath();
    FILE *fp = fopen(p, "w");
    for (size_t i = 0; i < v->l; i++)
        fprintf(fp, "%s\n", SVEC(v,i)->b);
    free(p);
    fclose(fp);
}

void
show_tasks(vec *v)
{
    for (int i = 0; i < v->l; i++)
    {
        char *p = SVEC(v,i)->b;
        printf("%d %s\n", i, p);
    }
}

int 
main(int argc,
        char **argv)
{
    vec *v = read_tasks();

    for (++argv;*argv;argv++)
        if (!strcmp(*argv, "add"))
            add_task(v, argv[1]);
        else if (!strcmp(*argv, "do"))
            do_task(v, strtol(argv[1], NULL, 10));
        else if (!strcmp(*argv, "clean"))
            clean_task(v);
        else if (!strcmp(*argv, "del"))
            del_task(v, strtol(argv[1], NULL, 10));
        else
            // print help
            ;
    show_tasks(v);
    write_tasks(v);
    vec_free(v);
    return 0;
}
