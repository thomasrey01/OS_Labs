#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"

/**
 * The function printChain prints a chain.
 * @param chain struct
 */
void printChain(struct chain *c)
{
    if (c->t == EXIT) {
        printf("exit\n");
        return;
    } else if (c->t == STATUS) {
        printf("status\n");
    }
    for (int i = 0; c->command[i] != NULL; i++) {
        printf("%s ", c->command[i]);
    }
    printf("\n");
}

/**
 * The function printOperator prints an operator.
 * @param Operator token enum.
 */
void printOperator(enum commOp op)
{
    switch (op) {
        case TREE_AND:
            printf(" && ");
            break;
        case TREE_OR:
            printf(" || ");
            break;
        case TREE_BACK:
            printf(" & ");
            break;
        case SEMI:
            printf(" ; ");
            break;
        default:
            break;
    }
}

/**
 * The function printSyntaxTree prints a syntax tree
 * @param syntax tree struct
 */
void printSyntaxTree(struct ast *tree)
{
    if (tree == NULL) {
        return;
    }
    switch (tree->t) {
        case INPUTLINE:
            printf("inputline: ");
            printSyntaxTree(tree->i->left);
            printOperator(tree->i->op);
            printSyntaxTree(tree->i->right);
            break;
        case PIPELINE:
            printf("pipeline: ");
            break;
        case CHAIN:
            printf("chain: ");
            printChain(tree->c);
            break;
        default:
            break;
    }
}

/**
 * The function freeSyntaxTree frees a syntax tree.
 * @param syntax tree struct
 */
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
        if (tree->c->command != NULL) {
            for (int i = 0; i < tree->c->size; i++) {
                if (tree->c->command[i] != NULL) {
                    free(tree->c->command[i]);
                }
            }
            free(tree->c->command);
        }
        free(tree->c);

    }
    free(tree);
}

/**
 * The function addNull adds a NULL pointer to the end of a chain command array.
 * @param chain struct pointer.
 */
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

/**
 * The function addCommand adds a command to a chain command array.
 * @param char s representing the command to add, struct chain *tree pointer 
 * to a chain struct
 */
void addCommand(char *s, struct chain *tree)
{
    if (tree->command == NULL) {
        tree->command = (char **)malloc(tree->size * sizeof(char *));
        for (int i = 0; i < tree->size; i++) {
            tree->command[i] = NULL;
        }
    }
    if (tree->ptr + 1 == tree->size) {
        int newSize = tree->size * 2;
        tree->command = realloc(tree->command, newSize * sizeof(char *));
        for (int i = tree->ptr+1; i < newSize; i++) {
            tree->command[i] = NULL;
        }
    }
    if (s == NULL) {
        tree->command[tree->ptr] = NULL;
        tree->ptr++;
        return;
    }
    tree->command[tree->ptr] = malloc((strlen(s) + 1) * sizeof(char));
    strcpy(tree->command[tree->ptr], s);
    tree->ptr++;
}

/**
 * The function createNode creates an abstract syntax tree node.
 * @param type enum token
 * @return abstract syntax tree node.
 */
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
            a->c->ptr = 0;
            a->c->size = 10;
            a->c->command = NULL;
            break;
        default:
            break;
    }
    return a;
}

/**
 * The function addOperator adds an operator to a tree node.
 * @param pointer to tree node, char array of operator
 */
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
