#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "scanner.h"
#include "ast.h"
#include "commands.h"
#include "parser.h"

int main(int argc, char *argv[]) {
    char *inputLine;
    List tokenList;

    //TODO: Signal back that the loop must stop when "exit" has been encountered (or EOF)
    while (true) {
        inputLine = readInputLine();
        tokenList = getTokenList(inputLine);
        printList(tokenList);
        // List tokenListCopy = tokenList;
        int status;
        struct ast *tree = parseInputLine(&tokenList, &status);
        int command;
        if (status != 0) {
            // Input was parsed successfully and can be accessed in "tokenList"

            printSyntaxTree(tree);
            break;
            // However, this is still a simple list of strings, it might be convenient
            // to build some intermediate structure representing the input line or a
            // command that you then construct in the parsing logic. It's up to you
            // to determine how to approach this!
            //  command = executeCommand(tree);
            // if (command == 0) {
            //    break;
            // }
        } else {
            printf("Error: invalid syntax!\n");
        }

        free(inputLine);
        freeTokenList(tokenList);
    }

    return 0;
}
