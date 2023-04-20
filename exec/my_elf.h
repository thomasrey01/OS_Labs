#ifndef ELF_H
#define ELF_H

#include <elf.h>
#include <stdlib.h>

struct ELF_image {
	Elf64_Ehdr header;
	Elf64_Phdr *p_headers;
	uint64_t entry;
};

int elf_sanity_check(Elf64_Ehdr *hd);

void elf_header_dump(const Elf64_Ehdr *hd);
void elf_program_header_dump(const Elf64_Phdr *ph, size_t n);

#endif
