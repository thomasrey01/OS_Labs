#include <stdbool.h>
#include <string.h>

#include "scanner.h"
#include "ast.h"

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

bool parseExecutable(List *lp) {

    char *s = (*lp)->t;
    bool canEnd = false;
    // TODO: Determine whether to accept parsing an executable here.
    // 
    // It is not recommended to check for existence of the executable already
    // here, since then it'll be hard to continue parsing the rest of the input
    // line (command execution should continue after a "not found" command),
    // it'll also be harder to print the correct error message.
    // 
    // Instead, we recommend to just do a syntactical check here, which makes
    // more sense, and defer the binary existence check to the runtime part
    // you'll write later.
    char c;
    for (int i = 0; (c = s[i]) != '\0'; i++) {
        if (isSpecialChar(c)) return false;
        if (c == '/') {
            if (!canEnd) return false;
            canEnd = false;
        } else {
            canEnd = true;
        }
    }
    return canEnd;
}

int parseBuiltIn(List *lp) // this is wrong fix this
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
            return i; 
        }
    }
    return -1;
}

/*  Here we only parse the chain either as a (built in or executable) and options
 *  <command> := <executable> <options>
 *             | <builtin> <option>
 *  executables can be defined by a path and options
 */

bool isOperator(char *s) {
    // NULL-terminated array makes it easy to expand this array later
    // without changing the code at other places.
    char *operators[] = {
            "&",
            "&&",
            "||",
            ";",
            "<",
            ">",
            "|",
            NULL
    };

    for (int i = 0; operators[i] != NULL; i++) {
        if (strcmp(s, operators[i]) == 0) return true;
    }
    return false;
}

bool parseOptions(List *lp)
{
    if (lp == NULL) {
        return false;
    }
    char *s = (*lp)->t;
    if (s == NULL || s[0] != '-') {
        return false;
    }
    return true;
}

struct ast *parseChain(List *lp, int *status)
{
    int i;
    if ((i = parseBuiltIn(lp)) != -1) {
        *status = 1;
        struct ast *tree = createNode(CHAIN);
        switch (i) {
            case 0:
                tree->c->t = EXIT;
                break;
            case 1:
                tree->c->t = STATUS;
                break;
            default:
                break;
        }
        return tree;
    } 
    if (parseExecutable(lp)) {
        *status = 1;
        struct ast *tree = createNode(CHAIN);
        addCommand((*lp)->t, tree->c);
        *lp = (*lp)->next;
        while (parseOptions(lp)) {
            addCommand((*lp)->t, tree->c);
            *lp = (*lp)->next;
        }
        addNull(tree->c);
        return tree;
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
        freeSyntaxTree(tree);
        return NULL;
    }
    enum commOp oper = parseOp(lp);
    if (oper != NONE) {
        tree->i->right = parseInputLine(lp, &stat);  
    } else {
        tree->i->right = NULL;
    }
    return tree;
}
