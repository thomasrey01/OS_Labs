#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "commands.h"

int status;
extern char **command;

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
    } else {
        wait(&status);
    }
    return;
}

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
                if (status == 0) {
                    commType = executeTree(tree->i->right);
                }
                break;
            case TREE_OR:
                if (status == 0) {
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
                printf("The most recent error code is: %d\n", status);
                commType = 0;
                break;
            case EXIT:
                commType = 0;
                break;
            case COMMAND:
                executeCommand(c->command); 
                commType = 1;
                break;
            default:
                break;
        }
    }
    return commType;
}
