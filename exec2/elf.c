#include "helper.h"
#include "my_elf.h"
#include "their_exec.h"

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

int elf_sanity_check(Elf64_Ehdr *hd)
{
	if (memcmp(hd->e_ident, ELFMAG, SELFMAG) != 0) {
		(void)fputs("Not a valid ELF file\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (hd->e_ident[EI_CLASS] != ELFCLASS64) {
		(void)fputs("Only accepting 64 bit ELF files", stderr);
		exit(EXIT_FAILURE);
	}
	switch (hd->e_ident[EI_DATA]) {
	case ELFDATA2LSB:
	case ELFDATA2MSB:
		break;
	default:
		(void)fputs("Invalid encoding", stderr);
		exit(EXIT_FAILURE);
	}
	if (hd->e_ident[EI_VERSION] != EV_CURRENT) {
		(void)fputs("Invalid ELF version", stderr);
		exit(EXIT_FAILURE);
	}
	switch (hd->e_ident[EI_OSABI]) {
	case ELFOSABI_GNU:
	case ELFOSABI_SYSV:
		break;
	case ELFOSABI_HPUX:
	case ELFOSABI_NETBSD:
	case ELFOSABI_SOLARIS:
	case ELFOSABI_AIX:
	case ELFOSABI_IRIX:
	case ELFOSABI_FREEBSD:
	case ELFOSABI_TRU64:
	case ELFOSABI_MODESTO:
	case ELFOSABI_OPENBSD:
	case ELFOSABI_ARM_AEABI:
	case ELFOSABI_ARM:
	case ELFOSABI_STANDALONE:
	default:
		(void)fputs("Invalid ABI", stderr);
		exit(EXIT_FAILURE);
	}
	if (hd->e_ident[EI_ABIVERSION] != 0) {
		(void)fputs("Invalid ABI", stderr);
		exit(EXIT_FAILURE);
	}
	if (hd->e_ident[EI_PAD] != 0) {
		(void)fputs("Likely out of date scanner", stderr);
		exit(EXIT_FAILURE);
	}

	switch (hd->e_type) {
	case ET_EXEC:
	case ET_DYN:
		break;
	case ET_REL:
	case ET_CORE:
		(void)fputs("Not dealing with REL or CORE ELF files\n", stderr);
		exit(EXIT_FAILURE);
	default:
		(void)fputs("Unkown ELF type", stderr);
		exit(EXIT_FAILURE);
	}

	if (hd->e_machine != EM_X86_64) {
		(void)fputs("Only accepting x64 elf files", stderr);
		exit(EXIT_FAILURE);
	}

	return 0;
}

