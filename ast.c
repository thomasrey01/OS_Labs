#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"

struct ast *createNode(enum type t)
{
    struct ast *a = (struct ast*)malloc(sizeof(struct ast));
    a->t = t;
    switch(t) {
        case INPUTLINE:
            a->i = (struct inputline*)malloc(sizeof(struct inputline));
            break;
        case CHAIN:
            a->c = (struct chain*)malloc(sizeof(struct chain));
            break;
        default:
            break;
    }
    return a;
}

void addOperator(struct ast *tree, char *op)
{   
    assert(tree->t == INPUTLINE);
    enum commOp treeOp;
    if (strcmp(op, "&&") == 0) {
        treeOp = TREE_AND;
    } else if (strcmp(op, "||") == 0) {
        treeOp = TREE_OR;
    } else if (strcmp(op, "&") == 0) {
        treeOp = TREE_BACK;
    } else if (strcmp(op, ";") == 0) {
        treeOp = SEMI;
    } else {
        printf("error, wrong operator\n");
        exit(1);
    }
    tree->i->op = treeOp;
}
