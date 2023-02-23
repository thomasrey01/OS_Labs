#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char *s;
    char t[] = "poop";
    s = malloc(strlen(t) * sizeof(char));
    strcpy(s, t);
    printf("%s\n", s);
}
