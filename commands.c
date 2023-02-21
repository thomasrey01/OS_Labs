#include "commands.h"

int executeCommand(List tokenList)
{
    char *s = tokenList->t;
    if (strcmp(s, "exit") == 0) {
        return 0;
    }
    if (strcmp(s, "status") == 0) {
        // IDK what this is supposed to do yet
    }
    return 1;
}
