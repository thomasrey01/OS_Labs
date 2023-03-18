#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "commands.h"

int status = 0;
int lastStatus = 0;

/**
 * This function executes the cd builtin command from a char array
 * @param char pointer
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

/**
 * This function executes a pipeline of multiple commands.
 * @param pipeline node and redirect node
 */
void executePipeline(struct pipeline *pipel, struct redirect *red)
{
    int in = 0;
    int fd[2];
    int size = 0;
    pid_t pid;
    struct pipeline *tmp = pipel;
    while (tmp != NULL) {
        size++;
        tmp = tmp->next;
    }

    if (pipe(fd) < 0) {
        exit(1);
    }

    for (int i = 0; i < size - 1; i++) {

        pid = fork();

        if (pid == -1) {
            fprintf(stderr, "fork error\n");
            exit(1);
        }
        if (pid == 0) {


            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);

            if (execvp(pipel->comm->command[0], pipel->comm->command) == -1){
                if (errno == 2) {
                    lastStatus = 127;
                    printf("Error: command not found!\n");
                }
            }
            exit(errno);
        } else {
            wait(&status);
            close(fd[1]);
        }
    }
    pid = fork();
    if (pid == -1) {
        exit(1);
    }
    if (pid == 0) {

        dup2(STDIN_FILENO, fd[1]);
        close(fd[1]);
        close(fd[0]);
    
        if (execvp(pipel->comm->command[0], pipel->comm->command) == -1) {
            if (errno == 2) {
                lastStatus = 127;
                printf("Error: command not found!\n");
            }
        }
        exit(errno);
    } else {
        wait(&status);
        close(fd[1]);
    }
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
                executePipeline(c->pipel, c->red);
                lastStatus = WEXITSTATUS(status);
                commType = 1;
                break;
            default:
                break;
        }
    }
    return commType;
}
