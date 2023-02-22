all: shell

shell:
	gcc -std=c11 -Wall -pedantic main.c ast.c arr.c commands.c scanner.c shell.c -o shell

clean:
	rm -f *~
	rm -f *.o
	rm -f shell
