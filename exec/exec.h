#ifndef EXEC_H
#define EXEC_H

#include <elf.h>

int my_exec(int argc, char **argv, char **env, Elf64_auxv_t *auxv);

#endif
