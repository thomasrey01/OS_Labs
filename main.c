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
        bool parsedSuccessfully = parseInputLine(&tokenList);
        
        if (tokenList == NULL && parsedSuccessfully) {
            exec_command(tokenList);
        } else {
            printf("Error: invalid syntax!\n");
        }
        
        free(inputLine);
        freeTokenList(tokenList);
    }
    
    return 0;
}
