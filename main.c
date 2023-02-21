#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "scanner.h"
#include "shell.h"
#include "commands.h"

int main(int argc, char *argv[]) {
    char *inputLine;
    List tokenList;

    //TODO: Signal back that the loop must stop when "exit" has been encountered (or EOF)
    while (true) {
        inputLine = readInputLine();
        tokenList = getTokenList(inputLine);
        // printList(tokenList);
        List tokenListCopy = tokenList;

        bool parsedSuccessfully = parseInputLine(&tokenListCopy);
        int command;
        if (tokenListCopy == NULL && parsedSuccessfully) {
            // Input was parsed successfully and can be accessed in "tokenList"

            // However, this is still a simple list of strings, it might be convenient
            // to build some intermediate structure representing the input line or a
            // command that you then construct in the parsing logic. It's up to you
            // to determine how to approach this!
            command = executeCommand(tokenList);
            if (command == 0) {
                break;
            }
        } else {
            printf("Error: invalid syntax!\n");
        }

        free(inputLine);
        freeTokenList(tokenList);
    }

    return 0;
}
