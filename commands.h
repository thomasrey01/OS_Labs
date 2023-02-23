#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "scanner.h"

int executeCommand(List tokenList);

#endif
