#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"

void printSyntaxTree(struct ast *tree)
{
    if (tree == NULL) {
        return;
    }
    switch (tree->t) {
        case INPUTLINE:
            printf("inputline: ");
            break;
        case PIPELINE:
            printf("pipeline: ");
            break;
        case CHAIN:
            printf("chain: ");
            break;
        default:
            break;
    }
}

void freeSyntaxTree(struct ast *tree)
{
    if (tree == NULL) {
        return;
    }
    if (tree->t == INPUTLINE) {
        freeSyntaxTree(tree->i->left);
        freeSyntaxTree(tree->i->right);
        free(tree->i);
    } else {
        free(tree->c);
    }
    free(tree);
}

void addNull(struct chain *tree)
{
    if (tree->command == NULL) {
        tree->command = (char **)malloc(tree->size * sizeof(char *));
    }
    if (tree->ptr + 1 == tree->size) {
        tree->size *= 2;
        tree->command = realloc(tree->command, tree->size * sizeof(char *));
    }
    tree->command[tree->ptr] = NULL;
    tree->ptr++;
}

void addCommand(char *s, struct chain *tree)
{
    if (tree->command == NULL) {
        tree->command = (char **)malloc(tree->size * sizeof(char *));
    }
    if (tree->ptr + 1 == tree->size) {
        tree->size *= 2;
        tree->command = realloc(tree->command, tree->size * sizeof(char *));
    }
    strcpy(tree->command[tree->ptr], s);
    tree->ptr++;
}

struct ast *createNode(enum type t)
{
    struct ast *a = (struct ast*)malloc(sizeof(struct ast));
    a->t = t;
    switch(t) {
        case INPUTLINE:
            a->i = (struct inputline*)malloc(sizeof(struct inputline));
            a->i->left = NULL;
            a->i->right = NULL;
            break;
        case CHAIN:
            a->c = (struct chain*)malloc(sizeof(struct chain));
            a->c->size = 10;
            a->c->ptr = 0;
            a->c->command = (char **)malloc(a->c->size * sizeof(char**));
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
