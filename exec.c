#include "exec.h"
#include "elf.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int is_dynamic = 0;
uint64_t interp;

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

unsigned char *get_loads(FILE *ptr, struct elf_header elf_head)
{
    struct elf_proghdr proghdr;
    struct stat file_info;
    fseek(ptr, 0, SEEK_SET);
    if (fstat(fileno(ptr), &file_info) == -1)
        exit(1);
    unsigned char *whole_elf =
        mmap(NULL, file_info.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
                fileno(ptr), 0);
    if (whole_elf == MAP_FAILED) {
        int err = errno;
        printf("%s\n", strerror(err));
    }
    unsigned char *loads =
        mmap(NULL, file_info.st_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    memset(loads, 0, file_info.st_size);
    int offset;
    int prot;
    for (int i = 0; i < elf_head.proghdr_count; i++) {
        offset = elf_head.proghdr_offset + i * elf_head.proghdr_size;
        fseek(ptr, offset, SEEK_SET);
        fread(&proghdr, sizeof(struct elf_proghdr), 1, ptr);
        if (proghdr.type == ELF_PROG_LOAD) {
            uint64_t page_size = sysconf(_SC_PAGE_SIZE);

            memmove(loads + proghdr.virtual_address,
                    whole_elf + proghdr.file_offset,
                    proghdr.size_file);
            uint64_t prot_offset =
                (uint64_t)(loads + proghdr.virtual_address) %
                page_size;
            prot = 0;
            if (proghdr.flags & ELF_PROG_FLAG_EXEC)
                prot |= PROT_EXEC;
            if (proghdr.flags & ELF_PROG_FLAG_WRITE)
                prot |= PROT_WRITE;
            if (proghdr.flags & ELF_PROG_FLAG_READ)
                prot |= PROT_READ;

            mprotect(loads + proghdr.file_offset - prot_offset,
                    proghdr.size_file + prot_offset, prot);
        }
    }
    return loads;
}

int main(int argc, char *argv[], char **envp)
{
    setbuf(stdout, NULL);
    if (argc < 2) {
        printf("Usage: %s <binary> [arguments] \n", argv[0]);
        exit(1);
    }
    FILE *ptr;
    ptr = fopen(argv[1], "rb");

    if (ptr == NULL) {
        printf("Error in opening file %s\n", argv[1]);
        exit(1);
    }
    printf("Opening binary %s\n", argv[1]);

    struct elf_header main_header = sanity_check(ptr);

    unsigned char *loads = get_loads(ptr, main_header);
    fseek(ptr, 0, SEEK_SET);
    uint64_t sz	     = ftell(ptr);
    const int size_stack = 4096 * 64; // random stack size

    uint64_t *stack = alloca(size_stack * sizeof(uint64_t));
    memset(stack, 0, size_stack * sizeof(uint64_t));

    int front = 0;
    int back  = size_stack - 1;

    stack[back] = (uint64_t)NULL;
    back--;
    stack[front] = argc - 1;
    front++;

    // loading arguments variables with strings copied in the back and
    // pointers in the front
    char *string_ptr = (char *)stack + back;
    argv++;
    while (*argv != NULL) {
        string_ptr -= strlen(*argv) + 1;
        stack[front] = (uint64_t)string_ptr;
        front++;
        //printf("copying to %p\n", string_ptr);
        strcpy(string_ptr, *argv);
        argv++;
    }

    stack[front] = (uint64_t)NULL;
    front++;
    for (; *envp; envp++) {
        string_ptr -= strlen(*envp) + 1;
        stack[front] = (uint64_t)string_ptr;
        //printf("copying to %p\n", string_ptr);
        strcpy(string_ptr, *envp);
        front++;
    }
    //printf("Writing NULL to %p\n", &stack[front]);
    stack[front++] = (uint64_t)NULL;
    envp++;
    struct elf_auxvec *auxvec = (struct elf_auxvec *)envp;
    while (auxvec->type != AT_NULL) {
        switch (auxvec->type) {
            case AT_NULL: break;
            case AT_IGNORE: break;
            case AT_PHDR:
                auxvec->value = (uint64_t)(loads + main_header.proghdr_offset);
                break;
            case AT_PHNUM:
                auxvec->value = (uint64_t)main_header.proghdr_count;
                break;
            case AT_ENTRY:
                auxvec->value = (uint64_t)(loads + main_header.entry);
                break;
            case AT_BASE:
                if (main_header.type == 3) auxvec->value = interp;
                break;
            default:
                break;
        }
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
