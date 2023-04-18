#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

struct descriptors {
    int in;
    int out;
};

void executeCommand(char **command);
int executeTree(struct ast *tree);

#endif
