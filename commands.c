#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "commands.h"

int exit_code = 0;

void exec_command(char **commands) {
    if (strcmp(commands[0], "exit") == 0) {
        exit(0);
    }
     if (strcmp(commands[0], "status") == 0) {
        printf("%d\n", exit_code);
        return;
     }
    
    pid_t child_pid = fork();

    if (child_pid == 0) {
        if(execvp(commands[0], commands) == -1) {
            if (errno == ENOENT) {
                fprintf(stderr, "Error: command not found!\n");
            }
        }
        exit(1);
    } else if (child_pid > 0) {
        int status;
        waitpid(child_pid, &status, 0);
        if(WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        }
    } else {
        printf("Error in creating child process;\n");
    }

}
