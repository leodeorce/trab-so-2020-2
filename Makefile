all: comp

comp: vsh.c lista.c
	gcc -o vsh vsh.c lista.c

run:
	./vsh

clean:
	rm -f vsh