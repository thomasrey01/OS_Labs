#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "commands.h"

void exec_command(char **commands) {
    if (strcmp(commands[0], "exit") == 0) {
        exit(0);
    }
    if (strcmp(commands[0], "status") == 0) {
        //TO DO: implement the status
    }
    
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
        perror("Error in creating child process;\n");
    }

}
