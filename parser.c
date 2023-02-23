#include "ast.h"
#include "scanner.h"

#include <stdbool.h>
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

bool isSpecialChar(char s) 
{
    return (s == '?' || s == '+' || s == '!' || s == '$' || s == '[');    
}

struct ast *parseExecutable(List *lp, int *status) 
{
    char *s = (*lp)->t;
    bool canEnd = false;
    char c;
    for (int i = 0; (c = s[i]) != '\0'; i++) {
        if (isSpecialChar(c)) {
            canEnd = false;
            break;
        }
        if (c == '/') {
            if (!canEnd) break;
            canEnd = false;
        } else {
            canEnd = true;
        }
    }
    if (canEnd) {
        *status = 1;
        struct chain *c = (struct chain*)malloc(struct chain);
    }
    *status = 0;
    return canEnd;
}

struct ast *parseBuiltIn(List *lp, int *status) // this is wrong fix this
{

    //
    // TODO: Implement the logic for these builtins, and extend with
    // more builtins down the line
    //

    // NULL-terminated array makes it easy to expand this array later
    // without changing the code at other places.
    char *builtIns[] = {
            "exit",
            "status",
            NULL
    };

    for (int i = 0; builtIns[i] != NULL; i++) {
        if (strcmp((*lp)->t, builtIns[i]) == 0) {
            *status = 1;
            struct ast *tree = (struct ast*)malloc(sizeof(struct ast));
            tree->t = CHAIN;
            tree->c = (struct chain*)malloc(sizeof(struct chain));
            strcpy(tree->c->command, (*lp)->t);
            return tree;
        }
    }
    *status = 0;
    return NULL;
}

/*  Here we only parse the chain either as a (built in or executable) and options
 *  <command> := <executable> <options>
 *             | <builtin> <option>
 *  executables can be defined by a path and options
 */

struct ast *parseChain(List *lp, int *status)
{
    if (parseBuiltin(lp) && parseOptions(lp)) {
        *status = 1;
    } 
    if (parseCommand(lp) && parseOptions(lp)) {
        *status = 1;
        struct ast *tree = createNode(CHAIN);
        // add  
    }
    *status = 0;
    return NULL;
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
