#
#  Integrantes:
#      Leonardo Deorce Lima de Oliveira
#      Maria Julia Damasceno
#      Pedro Victor Alves de Souza Santos
#

all: comp

comp: vsh_alt.c lista.c
	gcc -Wall -std=gnu99 vsh_alt.c lista.c -o vsh

run:
	./vsh

run-v:
	valgrind -s --leak-check=full --show-leak-kinds=all ./vsh

clean:
	rm -f vsh