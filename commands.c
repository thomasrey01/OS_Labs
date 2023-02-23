#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "commands.h"

extern int last_operation;

void exec_command(char **commands) {
    if (strcmp(commands[0], "exit") == 0) {
        exit(0);
    }
    //  if (strcmp(commands[0], "status") == 0) {
    //     printf("%d\n", last_operation);
    //  }
    
    pid_t child_pid = fork();

    if (child_pid == 0) {
        execvp(commands[0], commands);
        exit(1);
    } else if (child_pid > 0) {
        int status;
        do {
            waitpid(child_pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
        printf("Error in creating child process;\n");
    }

}
