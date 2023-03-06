#include <stdbool.h>
#include <string.h>

#include "scanner.h"
#include "ast.h"

/** 
 * Function to get boolean operator from token
 * @param lp representing a pointer to a List
 * @return an enum representing the boolean operator.
 */

enum commOp parseOp(List *lp)
{
    if (*lp == NULL) {
        return NONE;
    }
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
    }
    return NONE;
}

/**
 * Function to check whether semicolon is at the current token
 * @param lp representing a pointer to a List
 * @return an enum representing the semicolon operator or one representing nothing
 */
enum commOp parseSemi(List *lp)
{
    if (*lp == NULL) {
        return NONE;
    }
    char *s = (*lp)->t;
    if (strcmp(s, ";") == 0) {
        *lp = (*lp)->next;
        return SEMI;
    } 
    return NONE;
}

/**
 * Checks whether char is a special character
 * @param char s
 * @return boolean value representing whether s is a special character
 */
bool isSpecialChar(char s) 
{
    return (s == '?' || s == '+' || s == '!' || s == '$' || s == '[');    
}

/** The function parseExecutable parses an executable.
 * @param lp List pointer to the start of the tokenlist.
 * @return a bool denoting whether the executable was parsed successfully.
 */
bool parseExecutable(List *lp) {

    char *s = (*lp)->t;
    bool canEnd = false;
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

/** The function parseBuiltin parses a builtin.
 * @param lp List pointer to the start of the tokenlist.
 * @return a bool denoting whether the builtin was parsed successfully.
 */
int parseBuiltIn(List *lp)
{
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

/**
 * Checks whether the input string \param s is an operator.
 * @param s input string.
 * @return a bool denoting whether the current string is an operator.
 */
bool isOperator(char *s)
{
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

/**
 * The function parseOptions parses options.
 * @param lp List pointer to the start of the tokenlist.
 * @return a bool denoting whether the options were parsed successfully.
 */
bool parseOptions(List *lp)
{
    if (*lp == NULL || isOperator((*lp)->t)) {
        return false;
    }
    return true;
}

/**
 * The function parseChain parses a chain and builds its node to the syntax tree.
 * @param lp List pointer to the start of the tokenlist, int *status pointer to an int
 * @return abstract syntax tree node.
 */
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
        *lp = (*lp)->next;
        return tree;
    } 
    if (parseExecutable(lp)) {
        *status = 1;
        struct ast *tree = createNode(CHAIN);
        tree->c->t = COMMAND;
        addCommand((*lp)->t, tree->c);
        *lp = (*lp)->next;
        while (parseOptions(lp)) {
            addCommand((*lp)->t, tree->c);
            *lp = (*lp)->next;
        }
        addNull(tree->c);
        return tree;
    }
    return NULL;
}

/**
 * The function parseInputLine parses an inputLine and builds its node to the syntax tree according to the grammar:
 * <inputline>      ::= <chain> & <inputline>
 *                   | <chain> && <inputline>
 *                   | <chain> || <inputline>
 *                   | <chain>
 *                   | <empty>
 * @param lp List pointer to the start of the tokenlist, int *status pointer to an int
 * @return abstract syntax tree node.
 */
struct ast *parseInputLine(List *lp, int *status)
{
    int stat;
    if (isEmpty(*lp)) {
        *status = 1;
        return NULL;
    }
    struct ast *tree = createNode(INPUTLINE);
    tree->i->left = parseChain(lp, &stat);
    if (stat == 0) {
        *status = 0;
        freeSyntaxTree(tree);
        return NULL;
    }
    enum commOp oper = parseOp(lp);
    tree->i->op = oper;
    if (oper != NONE && oper != SEMI) {
        tree->i->right = parseInputLine(lp, &stat);  
    } else {
        tree->i->right = NULL;
    }
    *status = 1;
    return tree;
}

/**
 * The function parseInputLine parses an a semiLine and builds its node to the syntax tree according to the grammar:
 * <semiLine>      ::= <inputLine> ; <semiLine>
 *                   | <inputLine>
 * @param lp List pointer to the start of the tokenlist, int *status pointer to an int
 * @return abstract syntax tree node.
 */
struct ast *parseSemiLine(List *lp, int *status)
{
    int stat;
    if (isEmpty(*lp)) {
        *status = 1;
        return NULL;
    }
    struct ast *tree = createNode(INPUTLINE);
    tree->i->left = parseInputLine(lp, &stat);
    if (stat == 0) {
        *status = 0;
        freeSyntaxTree(tree);
        return NULL;
    }
    enum commOp oper = parseSemi(lp);
    tree->i->op = oper;
    if (oper == SEMI) {
        tree->i->right = parseSemiLine(lp, &stat);
    } else {
        tree->i->right = NULL;
    }
    *status = 1;
    return tree;
}
