#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

int main(int argc, char **argv) {
    int fd;
    Elf64_Ehdr elf_header;
    Elf64_Phdr prog_header;
    void *mem;
    size_t i;
    off_t offset;
    size_t size;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* Read ELF header */
    if (read(fd, &elf_header, sizeof(Elf64_Ehdr)) != sizeof(Elf64_Ehdr)) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    /* Calculate total size of LOAD segments */
    size = 0;
    for (i = 0; i < elf_header.e_phnum; i++) {
        offset = elf_header.e_phoff + i * elf_header.e_phentsize;
        if (lseek(fd, offset, SEEK_SET) != offset) {
            perror("lseek");
            exit(EXIT_FAILURE);
        }
        if (read(fd, &prog_header, sizeof(Elf64_Phdr)) != sizeof(Elf64_Phdr)) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (prog_header.p_type == PT_LOAD) {
            size += prog_header.p_memsz;
        }
    }

    /* Allocate contiguous memory */
    mem = mmap(mem, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    /* Load segments into memory */
    void *p = mem;
    for (i = 0; i < elf_header.e_phnum; i++) {
        offset = elf_header.e_phoff + i * elf_header.e_phentsize;
        if (lseek(fd, offset, SEEK_SET) != offset) {
            perror("lseek");
            exit(EXIT_FAILURE);
        }
        if (read(fd, &prog_header, sizeof(Elf64_Phdr)) != sizeof(Elf64_Phdr)) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (prog_header.p_type == PT_LOAD) {
            void *segment = mmap(p, prog_header.p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_FIXED, fd, prog_header.p_offset);
            if (segment == MAP_FAILED) {
                perror("mmap");
                exit(EXIT_FAILURE);
            }
            memcpy(p, segment, prog_header.p_filesz);
            p += prog_header.p_memsz;
            munmap(segment, prog_header.p_memsz);
        }
    }

    /* Set up stack */
    void *stack = alloca(4096);
    asm volatile("movq %0, %%rsp" : : "r"((char *)stack + 4096));

    /* Call entry point */
    ((void (*)(void)) elf_header.e_entry)();

    munmap(mem, size);
    close(fd);

    return 0;
}

