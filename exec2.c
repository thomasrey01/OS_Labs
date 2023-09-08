#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "elf.h"
#include "exec.h"

void stack_print(void *stack, size_t size)
{
	uint64_t *s1 = (uint64_t *)stack;

	printf("%p: %ld\n", s1, *s1);
	s1++;

	puts("argv:");
	while (*s1) {
		printf("%p: 0x%08lx\n", s1, *(uint64_t *)s1);
		s1++;
	}
	printf("%p: 0x%08lx\n", s1, *(uint64_t *)s1);
	s1++;

	puts("envp:");
	while (*s1) {
		printf("%p: 0x%08lx\n", s1, *(uint64_t *)s1);
		s1++;
	}
	printf("%p: 0x%08lx\n", s1, *(uint64_t *)s1);
	s1++;

	struct elf_auxvec *s2 = (struct elf_auxvec *)s1;

	puts("auxv:");
	while (*(uint64_t *)s2) {
		printf("%p: 0x%08lx 0x%08lx\n", s2, s2->type, s2->value);
		s2++;
	}
	printf("%p: 0x%08lx\n", s2, *(uint64_t *)s2);

	unsigned char *s3 = (unsigned char *)s2;

	while (!*s3++)
		;

	puts("strings:");
	printf("\n%p: ", s1);
	for (; s3 < (unsigned char *)stack + size - sizeof(uint64_t); s3++) {
		if ((uint64_t)s3 % sizeof(uint64_t) == 0)
			printf("\n%p: ", s3);
		putchar(*s3 > 32 && *s3 < 127 ? *s3 : '.');
	}

	printf("\n%p: 0x%08lx\n", s3, *(uint64_t *)s3);
	s3 += sizeof(uint64_t);
}

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

unsigned char *get_loads(FILE *ptr, struct elf_header elf_head)
{
    struct elf_proghdr proghdr;
    struct stat file_info;
    fseek(ptr, 0, SEEK_SET);
    if (fstat(fileno(ptr), &file_info) == -1) exit(1);
    unsigned char *whole_elf = mmap(NULL, file_info.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(ptr), 0);
    if (whole_elf == MAP_FAILED) {
        int err = errno;
        printf("%s\n", strerror(err));
    }
    unsigned char *loads = mmap(NULL, file_info.st_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    memset(loads, 0, file_info.st_size);
    int offset;
    int prot;
    for (int i = 0; i < elf_head.proghdr_count; i++) {
        offset = elf_head.proghdr_offset + i * elf_head.proghdr_size;
        fseek(ptr, offset, SEEK_SET);
        fread(&proghdr, sizeof(struct elf_proghdr), 1, ptr);
        if (proghdr.type == ELF_PROG_LOAD) {
            uint64_t page_size = sysconf(_SC_PAGE_SIZE);
            

            memmove(loads + proghdr.virtual_address, whole_elf + proghdr.file_offset, proghdr.size_file);
            uint64_t prot_offset = (uint64_t)(loads + proghdr.virtual_address) % page_size;
            prot = 0;
            if (proghdr.flags & ELF_PROG_FLAG_EXEC) prot |= PROT_EXEC;
            if (proghdr.flags & ELF_PROG_FLAG_WRITE) prot |= PROT_WRITE;
            if (proghdr.flags & ELF_PROG_FLAG_READ) prot |= PROT_READ;
            
            mprotect(loads + proghdr.file_offset - prot_offset, proghdr.size_file + prot_offset, prot);
        }
    }
    return loads;
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

    struct elf_header main_header = sanity_check(ptr);

    unsigned char *loads = get_loads(ptr, main_header);
    fseek(ptr, 0, SEEK_SET);
    uint64_t sz = ftell(ptr);
    const int size_stack = 4096; //random stack size

    uint64_t *stack = alloca(size_stack * sizeof(uint64_t));
    memset(stack, 0, size_stack * sizeof(uint64_t));

    int front = 0;
    int back = size_stack - 1;

    stack[back] = (uint64_t)NULL;
    back--;
    stack[front] = argc - 1;
    front++;

    // loading arguments variables with strings copied in the back and pointers in the front
    char *string_ptr = (char *)(stack + back);
    argv++;
    while (*argv != NULL) {
        string_ptr -= strlen(*argv) + 1;
        stack[front] = (uint64_t)string_ptr;
        front++;
        strcpy(string_ptr, *argv);
        argv++;
    }

    stack[front] = (uint64_t)NULL;
    front++;
    for (; *envp; envp++) {
        string_ptr -= strlen(*envp) + 1;
        stack[front] = (uint64_t)string_ptr;
        strcpy(string_ptr, *envp);
        front++;
    }
    stack[front] = (uint64_t)NULL;
    front++;
    envp++;
    struct elf_auxvec *auxvec = (struct elf_auxvec*)envp;
    while (auxvec->type != AT_NULL) {
        printf("type: %ld value: %ld\n", auxvec->type, auxvec->value);
        stack[front] = (uint64_t)auxvec->type;
        front++;
        stack[front] = (uint64_t)auxvec->value;
        front++;
        auxvec++;
    } 
    printf("here");
    stack_print(stack, size_stack);

    jump(loads + main_header.entry, stack);

    printf("Finishing up...\n");
    fclose(ptr);
    return 0;
}
