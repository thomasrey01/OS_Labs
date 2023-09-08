.PHONY: clean debug asan

CFLAGS= -Wall -Wextra -O2 -std=gnu17
DEBUG=0

exec: elf.c elf_dump.o helper.o exec.o
	gcc $(CFLAGS) $^ -lm -o $@

%.o: %.c %.h
	gcc $(CFLAGS) -c $<

debug: CFLAGS += -g -DDEBUG -O0 -fsanitize=undefined
debug: exec

asan: CFLAGS += -fsanitize=address
asan: debug

clean:
	rm -f *.o
	rm -f exec a.out
