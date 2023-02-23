#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

int executeCommand(struct ast *tree);

#endif
