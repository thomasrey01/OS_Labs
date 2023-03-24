#include <stdio.h>
#include <stdlib.h>
#include "elf.h"
#include "exec.h"

int main(int argc, char *argv[])
{
    struct elf_header *elf_head;
    if (argc != 2) {
        printf("Usage: %s <binary> [arguments] \n", argv[0]);
        exit(1);
    }
    FILE* ptr;
    ptr = fopen(argv[1], "rb");

    if (ptr == NULL) {
        printf("Error in opening file %s\n", argv[1]);
        exit(1);
    }
    fread(elf_head, sizeof(struct elf_header), 1, ptr);

    if (elf_head->magic != ELF_MAGIC ||
        elf_head->machine != ELF_MACHINE_x64 ||
        elf_head->version != ELF_VERSION) {
        printf("Invalid ELF file!\n");
        exit(1);
    }
    
    fclose(ptr);
    return 0;
}
