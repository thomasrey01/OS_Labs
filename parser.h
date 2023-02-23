#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "scanner.h"

struct ast *parseInputLine(List *lp, int *status);

#endif
