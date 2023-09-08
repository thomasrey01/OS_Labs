all:
	gcc exec.c -o exec 

debug:
	gcc -g exec.c

clean:
	rm -f a.out
	rm -f exec
