#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "scanner.h"
#include "shell.h"
#include "commands.h"

int main(int argc, char *argv[]) {
    char *inputLine;
    List tokenList, tokenListCopy;
    extern char *commands[COMMANDS_SIZE][COMMANDS_SIZE];
    extern int a;

    //TODO: Signal back that the loop must stop when "exit" has been encountered (or EOF)
    while (true) {
        inputLine = readInputLine();
        tokenList = getTokenList(inputLine);
        tokenListCopy = tokenList;

        bool parsedSuccessfully = parseInputLine(&tokenList);
        if (tokenList == NULL && parsedSuccessfully) {
            for(int j = 0; j < a; j++) {
                exec_command(commands[j]);
            }
            a = 0;
        } else {
            printf("Error: invalid syntax!\n");
        }

        free(inputLine);
        freeTokenList(tokenListCopy);
    }
    
    return 0;
}
