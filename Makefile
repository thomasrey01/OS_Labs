all: shell

shell:
	gcc -std=c11 -Wall -pedantic main.c ast.c scanner.c parser.c commands.c -o shell

debug:
	gcc -g -std=c11 -Wall -pedantic main.c ast.c scanner.c parser.c commands.c -o shell

clean:
	rm -f *~
	rm -f *.o
	rm -f shell
