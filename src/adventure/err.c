#include "err.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void print(char *type, char *s1, char *s2)
{
    (void)s1;
    (void)s2;
    fprintf(stderr, "%s: %s\n", type, strerror(errno));
}

void err(int eno, char *s1, char *s2)
{
    print("Error", s1, s2);
    exit(eno);
}

void warn(char *s1, char *s2)
{
    print("Warning", s1, s2);
}
