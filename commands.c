#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "commands.h"

int status = 0;
int lastStatus = 0;
extern char **command;

/**
 * This function executes a command and its parameters from a 2d char array
 * @param 2d char array
 */

void executeCD(char *command)
{
    if (command == NULL) {
        printf("Error: cd requires folder to navigate to!\n");
        lastStatus = 2;
        return;
    }
    if (chdir(command) == -1) {
        lastStatus = 2;
        printf("Error: cd directory not found!\n");
        return;
    }
    lastStatus = 0;
}

void executePipeline(struct pipeline *pipel)
{
    // for now this only executes the first one
    executeCommand(pipel->comm->command);
}

void executeCommand(char **s)
{
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork() could not create a child process!");
        exit(0);
    }
    else if (pid == 0) {
        execvp(s[0], s);
        exit(errno);
    } else {
        wait(&status);
    }
    return;
}

/**
 * Executes all commands in syntax tree
 *
 * @param abstract syntax stree struct
 * @returns int indicating whether exit has been called
 */

int executeTree(struct ast *tree)
{
    if (tree == NULL) {
        return 1;
    }
    int commType;
    if (tree->t == INPUTLINE) {
        commType = executeTree(tree->i->left);
        if (commType == 0) return 0;
        switch (tree->i->op) {
            case NONE:
                break;
            case TREE_AND:
                if (lastStatus == 0) {
                    commType = executeTree(tree->i->right);
                }
                break;
            case TREE_OR:
                if (lastStatus == 0) {
                    break;
                }
                commType = executeTree(tree->i->right);
                break;
            case SEMI:
                commType = executeTree(tree->i->right);
                break;
            default:
                break;
        }
    } else if (tree->t == CHAIN) {
        struct chain *c = tree->c;
        switch (c->t) {
            case STATUS:
                printf("The most recent exit code is: %d\n", lastStatus);
                commType = 1;
                break;
            case CD:
                executeCD(c->dir);
                commType = 1;
                break;
            case EXIT:
                commType = 0;
                break;
            case COMMAND:
                executePipeline(c->pipel);
                lastStatus = WEXITSTATUS(status);
                if (lastStatus == 127) { 
                    printf("Error: command not found!\n");
                }
                commType = 1;
                break;
            default:
                break;
        }
    }
    return commType;
}
