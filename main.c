#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "scanner.h"
#include "ast.h"
#include "commands.h"
#include "parser.h"

/** Main function. Calls readInputLine and calls functions to build tokenList and to build abstract syntax tree
 */

int main(int argc, char *argv[]) {
    char *inputLine;
    List tokenList;

    while (true) {
        inputLine = readInputLine();
        if (inputLine == NULL) {
            break;
        }
        tokenList = getTokenList(inputLine);
        List tokenListCopy = tokenList;
        int status;
        struct ast *tree = parseSemiLine(&tokenListCopy, &status);
        int command;
        if (status != 0) {
            command = executeTree(tree);
       } else {
            printf("Error: invalid syntax!\n");
        }
        free(inputLine);
        freeTokenList(tokenList);
        freeSyntaxTree(tree);
        if (command == 0) {
            break;
        }
    }
    return 0;
}
