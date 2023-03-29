#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <elf.h>

int main(int argc, char **argv) {
    FILE *file;
    Elf64_Ehdr elf_header;
    Elf64_Phdr prog_header;
    char *mem;
    size_t i;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    /* Read ELF header */
    fread(&elf_header, sizeof(Elf64_Ehdr), 1, file);

    /* Allocate memory for program segments */
    mem = malloc(elf_header.e_phnum * elf_header.e_phentsize);
    memset(mem, 0, elf_header.e_phnum * elf_header.e_phentsize);

    /* Read program headers */
    for (i = 0; i < elf_header.e_phnum; i++) {
        fseek(file, elf_header.e_phoff + i * elf_header.e_phentsize, SEEK_SET);
        fread(&prog_header, sizeof(Elf64_Phdr), 1, file);

        if (prog_header.p_type == PT_LOAD) {
            /* Copy load segment into memory */
            fseek(file, prog_header.p_offset, SEEK_SET);
            fread(mem + prog_header.p_vaddr, prog_header.p_memsz, 1, file);

            /* Set protection on memory */
            int prot = 0;
            if (prog_header.p_flags & PF_X) prot |= PROT_EXEC;
            if (prog_header.p_flags & PF_W) prot |= PROT_WRITE;
            if (prog_header.p_flags & PF_R) prot |= PROT_READ;
            if (mprotect(mem + prog_header.p_vaddr, prog_header.p_memsz, prot) == -1) {
                perror("mprotect");
                exit(EXIT_FAILURE);
            }
        }
    }

    /* Set up stack */
    void *stack = alloca(4096);
    asm volatile("movq %0, %%rsp" : : "r"((char *)stack + 4096));

    /* Call entry point */
    ((void (*)(void)) elf_header.e_entry)();

    free(mem);
    fclose(file);

    return 0;
}

