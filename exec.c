#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "elf.h"
#include "exec.h"

struct elf_header sanity_check(FILE *ptr)
{
    struct elf_header elf_head;
    fread(&elf_head, sizeof(struct elf_header), 1, ptr);
    if (elf_head.magic != ELF_MAGIC ||
            elf_head.machine != ELF_MACHINE_x64 ||
            elf_head.version != ELF_VERSION) {
        printf("Invalid ELF file!\n");
        exit(1);
    } 
    return elf_head;
}

void print_stack(uint64_t *stack)
{
    for (int i = 0; i < 1000; i++) {
        printf("%p: 0x%08lx\n", stack + i, stack[i]);
    }
}

void print_elf_header(struct elf_header head)
{
    printf("entry: %lx\n", head.entry);
    printf("program header counts: %d\n", head.proghdr_count);
    printf("program header size: %d\n", head.proghdr_size);
    printf("program header offset: %lx\n", head.proghdr_offset);
}

void print_section_header(int i, struct elf_proghdr proghdr)
{
    printf("%d: size: %lx\n", i, proghdr.size_file);
    printf("%d: offset: %lx\n", i, proghdr.file_offset);
}

/**
 *  *get_load function, loads the LOAD segments of an elf file in memory
 *  @param: pointer to beginning of file, current pointer to the file, elf file header
 *  @returns: unsigned char pointer to memory location
 */

unsigned char *get_load(FILE *initial_ptr, FILE *ptr, struct elf_header elf_head)
{
    struct elf_proghdr proghdr;
    int fd = fileno(ptr);
    fseek(ptr, 0, SEEK_END);
    long sz = ftell(ptr);
    void *load_segments = mmap(NULL, sz, PROT_READ | PROT_EXEC | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    memset(load_segments, 0, sz);
    unsigned char *mem_file = malloc(sz);
    fread(mem_file, sz, 1, initial_ptr);
    int prot;

    for (int i = 0; i < elf_head.proghdr_count; i++) {

        int offset = elf_head.proghdr_offset + i * elf_head.proghdr_size; //calculating the offset to get the next program headers
        fseek(ptr, offset, SEEK_SET);
        fread(&proghdr, sizeof(struct elf_proghdr), 1, ptr);

        if (proghdr.type == ELF_PROG_LOAD) {

            prot = 0;
            if (proghdr.flags & ELF_PROG_FLAG_EXEC) prot |= PROT_EXEC;
            if (proghdr.flags & ELF_PROG_FLAG_WRITE) prot |= PROT_WRITE;
            if (proghdr.flags & ELF_PROG_FLAG_READ) prot |= PROT_READ;

            memmove(load_segments + proghdr.virtual_address, mem_file + proghdr.file_offset, proghdr.size_file);

            mprotect(load_segments, proghdr.size_memory, prot);
        }

    }
    return load_segments;
}

int main(int argc, char *argv[], char **envp)
{
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
    printf("Opening binary %s\n", argv[1]);
    FILE* initial_ptr = ptr;

    struct elf_header main_header = sanity_check(ptr);

    unsigned char *loads = get_load(initial_ptr, ptr, main_header);
    const int size_stack = 1000; //random stack size

    uint64_t *stack = alloca(size_stack * sizeof(uint64_t));
    memset(stack, 0, size_stack * sizeof(uint64_t));

    int front = 0;
    int back = size_stack - 1;

    stack[back] = (uint64_t)NULL;
    back--;
    stack[front] = argc;
    front++;

    // loading arguments variables with strings copied in the back and pointers in the front
    char *string_ptr = (char *)(stack +back);
    while (*argv != NULL) {
        string_ptr -= strlen(*argv) + 1;
        stack[front] = (uint64_t)string_ptr;
        front++;
        strcpy(string_ptr, *argv);
        argv++;
    }

    stack[front] = (uint64_t)NULL;
    front++;
    while (*envp != NULL) {
        string_ptr -= strlen(*envp) + 1;
        stack[front] = (uint64_t)string_ptr;
        strcpy(string_ptr, *envp);
        front++;
        envp++;
    }
    stack[front] = (uint64_t)NULL;
    envp++;
    struct elf_auxvec *auxvec = (struct elf_auxvec*)envp;
    while (auxvec->type != AT_NULL) {
        stack[front] = auxvec->type;
        front++;
        stack[front] = auxvec->value;
        front++;
        auxvec++;
    } 

    jump(loads + main_header.entry, stack);

    printf("Finishing up...\n");
    fclose(ptr);
    return 0;
}
