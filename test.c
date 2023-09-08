#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        exit(1);
    }
    printf("%s %s %s\n", argv[0], argv[1], argv[2]);
}
