#include "helper.h"
#include "my_elf.h"
#include "their_exec.h"

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct Buffer {
	size_t len, pos;
	unsigned char b[];
};

int ul_exec(const char *path, char *const *argv, char *const *env)
{
	return 0;
}

/* Scaffolding code to guide the jump to the other binary */
// Finalize is called when the "child" exits
static void finalize()
{
	fflush(stdout);
	printf("Finishing up...\n");
}

// This function is used to perform the jump to the loaded binary
// It takes two pointers: the entry point of the new binary and a new stack
// pointer.
_Noreturn void jump(void *entry_point, void *stack_pointer)
{
	printf("Entry: %p\nStack: %p\n", entry_point, stack_pointer);
	// We have to do this in pure assembly; this should never return!
	// x86-64 ABI $3.4.1 states the following about registers for new
	// processes:
	// RBP is unspecified - so we can leave it
	// RSP should hold the stack pointer
	// RDX may hold a function pointer to a function that is called after
	// exit, or it may hold NULL. We choose the former, to verify proper
	// finish.
	asm volatile("mov %[fin], %%rdx"
		     "\n" // set the finalization function
		     "mov %[sp], %%rsp"
		     "\n"	  // set the stack pointer
		     "jmp *%[ep]" // jump to the entry point

		     // C machinery to pass C variables into assembly
		     ::[sp] "r"(stack_pointer),
		     [ep] "r"(entry_point), [fin] "r"(finalize)
		     : "rdx" // we exclude RDX from being used by the compiler
			     // for the code above
	);

	// We should never get here!
	printf("Error: jump failed or something strange happened\n");
	exit(1);
}

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

	Elf64_auxv_t *s2 = (Elf64_auxv_t *)s1;

	puts("auxv:");
	while (*(uint64_t *)s2) {
		printf("%p: 0x%08lx 0x%08lx\n", s2, s2->a_type, s2->a_un.a_val);
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

void *elf_load(const char *path, uint argc, char **argv, char **envp,
	       Elf64_auxv_t *auxv)
{
	FILE *fp = fopen(path, "rb");
	if (!fp)
		exit_with_error();

	struct stat file_info;
	if (fstat(fileno(fp), &file_info) == -1)
		exit_with_error();

	unsigned char *addr =
	    mmap(NULL, file_info.st_size, PROT_READ | PROT_EXEC, MAP_PRIVATE,
		 fileno(fp), 0);

	if (addr == MAP_FAILED) // NOLINT
		exit_with_error();
	printf("Addr: %p\n", addr);

	if (fclose(fp))
		exit_with_error();

	Elf64_Ehdr *elf_header = (Elf64_Ehdr *)addr;

	// elf_header_dump(elf_header);
	elf_sanity_check(elf_header);

	Elf64_Phdr *program_headers =
	    (Elf64_Phdr *)(addr + elf_header->e_phoff);

	// puts("\n");
	// elf_program_header_dump(program_headers, elf_header->e_phnum);
	// puts("\n");

	unsigned char *exec =
	    mmap(NULL, file_info.st_size, PROT_READ | PROT_WRITE | PROT_EXEC,
		 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (exec == MAP_FAILED) // NOLINT
		exit_with_error();
	memset(exec, 0, file_info.st_size);

	for (uint i = 0; i < elf_header->e_phnum; i++) {
		if (program_headers[i].p_type != PT_LOAD)
			continue;

		uint64_t ps  = sysconf(_SC_PAGE_SIZE);
		uint64_t loc = (uint64_t)(exec + program_headers[i].p_vaddr);
		printf("page_size: %ld\n location: %ld\n   modulo: %ld\n", ps,
		       loc, loc % ps);
		memmove(exec + program_headers[i].p_vaddr,
			addr + program_headers[i].p_offset,
			program_headers[i].p_filesz);

		uint64_t offset =
		    (uint64_t)(exec + program_headers[i].p_vaddr) %
		    sysconf(_SC_PAGE_SIZE);

		int flags = 0;

		if (program_headers[i].p_flags & PF_R)
			flags |= PROT_READ;
		if (program_headers[i].p_flags & PF_W)
			flags |= PROT_WRITE;
		if (program_headers[i].p_flags & PF_X)
			flags |= PROT_EXEC;

		if (mprotect(exec + program_headers[i].p_offset - offset,
			     program_headers[i].p_filesz + offset, flags) == -1)
			exit_with_error();
	}

	size_t stack_size = 512;

	uint64_t *stack = alloca(stack_size * sizeof(*stack));
	memset(stack, 0, stack_size * sizeof(*stack));
	printf("Stack: %p-%p\n", stack,
	       stack + stack_size + sizeof(uint64_t) - 1);
	size_t front = 0;
	size_t back  = stack_size - 1;

	stack[back--]  = (uint64_t)NULL;
	stack[front++] = argc;

	char *strings = (char *)stack + back;
	for (; *argv; argv++) {
		size_t len = strlen(*argv);
		strings -= len + 1;
		strcpy(strings, *argv);
		printf("argv: %p\n", stack + front);
		stack[front++] = (uint64_t)strings;
	}
	printf("NULL: %p\n", stack + front);
	stack[front++] = (uint64_t)NULL;
	for (; *envp; envp++) {
		size_t len = strlen(*envp);
		strings -= len + 1;
		strcpy(strings, *envp);
		printf("envp: %p\n", stack + front);
		stack[front++] = (uint64_t)strings;
	}
	printf("NULL: %p\n", stack + front);
	stack[front++] = (uint64_t)NULL;
	for (; auxv->a_type != AT_NULL; auxv++) {
		printf("auxv: %p\n", stack + front);
		stack[front++] = auxv->a_type;
		stack[front++] = auxv->a_un.a_val;
	}
	printf("NULL: %p\n", stack + front);
	stack[front++] = (uint64_t)NULL;

	stack_print(stack, stack_size * sizeof(uint64_t));

	jump(exec + elf_header->e_entry, stack);

	return 0;
}

void my_exec(int argc, char **argv, char **env, Elf64_auxv_t *auxv)
{
	elf_load(*argv, argc, argv, env, auxv);

	// ul_exec(path, argv, env);
	// if (execv(path, argv) == -1)
	// 	exit_with_error();
	exit(0);
}

int main(int argc, char **argv, char **envp)
{
	Elf64_auxv_t *auxv;
	uint i = 0;
	while (envp[i++])
		;

	auxv = (Elf64_auxv_t *)(envp + i);
	my_exec(argc - 1, argv + 1, envp, auxv);

	return 0;
}
