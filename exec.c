#include <stdio.h>
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

void *get_load(FILE *initial_ptr, FILE *ptr, struct elf_header elf_head)
{
    struct elf_proghdr proghdr;
    void *load_segments = malloc(elf_head.secthdr_size * elf_head.secthdr_count);
    memset(load_segments, 0, elf_head.secthdr_size * elf_head.secthdr_count);
    for (int i = 0; i < elf_head.proghdr_count; i++) {
        int offset = elf_head.proghdr_offset + i * elf_head.proghdr_size;
        fseek(ptr, offset, SEEK_SET);
        fread(&proghdr, sizeof(struct elf_proghdr), 1, ptr);

        if (proghdr.type == ELF_PROG_LOAD) {
            fseek(ptr, proghdr.file_offset, SEEK_SET);
            fread((char *)load_segments + proghdr.virtual_address, proghdr.size_memory, 1, ptr);
        }

    }
    return load_segments;
}

int main(int argc, char *argv[])
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
    FILE* initial_ptr = ptr;

    struct elf_header main_header = sanity_check(ptr);
    
    void *stack_ptr = alloca(10000); // random magic number because why not?
    void *loads = get_load(initial_ptr, ptr, main_header);
    jump(loads + main_header.entry, stack_ptr);

    printf("valid file\n");
    fclose(ptr);
    return 0;
}
