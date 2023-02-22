#include "commands.h"

int status;

int executeCommand(List tokenList)
{
    char *s = tokenList->t;
    if (strcmp(s, "exit") == 0) {
        return 0;
    }
    if (strcmp(s, "status") == 0) {
        // IDK what this is supposed to do yet
        if (globalToken != NULL) {
            printf("%s\n", globalToken->t);
        }
    }
    // status = execv() 
    return 1;
}
