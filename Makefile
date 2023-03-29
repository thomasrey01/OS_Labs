all:
	gcc exec.c 

debug:
	gcc -g exec.c

clean:
	rm -f a.out
