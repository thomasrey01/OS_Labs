#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "commands.h"

int status;
extern char **command;

int executeCommand(struct ast *tree)
{
    char *s = tokenList->t;
    if (strcmp(s, "exit") == 0) {
        return 0;
    }
    if (strcmp(s, "status") == 0) {
        // IDK what this is supposed to do yet
        // if (globalToken != NULL) {
        //     printf("%s\n", globalToken->t);
        // }
    }
    // status = execv() 
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        printf("Error in creating child process;\n");
        exit(0);
    }
    if (pid == 0) {
        status = execv(command[0], command);
        exit(0);
    } else {
        wait(NULL);
        return 1;
    }
    return 0;
}
