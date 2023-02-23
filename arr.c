#include "arr.h"
#include <stdlib.h>
#include <string.h>

char **command = NULL;
int size = 5;
int ptr = 0;



void addNull()
{
    if (command == NULL) {
        command = (char **)malloc(size * sizeof(char *));
    } if (ptr + 1 == size) {
        size *= 2;
        command = realloc(command, size * sizeof(char));
    }
    command[ptr] = NULL;
    ptr++;
}
