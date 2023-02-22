#include "ast.h"
#include "scanner.h"

#include <string.h>

enum commOp parseOp(List *lp)
{
    char *s = (*lp)->t;
    if (strcmp(s, "&") == 0) {
        *lp = (*lp)->next;
        return TREE_BACK;
    } else if (strcmp(s, "&&") == 0) {
        *lp = (*lp)->next;
        return TREE_AND;
    } else if (strcmp(s, "||") == 0) {
        *lp = (*lp)->next;
        return TREE_OR;
    } else if (strcmp(s, ";") == 0) {
        *lp = (*lp)->next;
        return SEMI;
    } 
    return NONE;
}

struct ast *parseChain(List *lp, int *status)
{
    
}

struct ast *parseInputLine(List *lp, int *status)
{
    struct ast *tree = createNode(INPUTLINE);
    int stat;
    if (isEmpty(*lp)) {
        *status = 1;
        return NULL;
    }
    tree->i->left = parseChain(lp, &stat);
    if (stat == 0) {
        *status = 0;
        return NULL;
    }
    enum commOp oper = parseOp(lp);
    if (oper != NONE) {
        tree->i->right = parseInputLine(lp, &stat);  
    }
    return tree;
}
