#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lista.h"

#define MAX_COMMAND_LENGTH 2097152

int main(void)
{
	// system("clear");
	char   token[MAX_COMMAND_LENGTH];
	char   charLido;
	int    index = 0;
	int    loop = 1;
	Token* listaTokens = listaInicializa();

	while(1) {

		printf("vsh> ");
		index = 0;
		loop = 1;

		while(loop == 1) {

			scanf("%c", &charLido);

			switch(charLido) {

				case '\n':
				case '\r':
					if(strlen(token) > 0) {
						printf(">%s<\n", token);
						listaTokens = listaInsere(token, listaTokens);
						listaImprime(listaTokens);
					}
					loop = 0;
					break;

				case '|':
					if(strlen(token) > 0) {
						// Mensagem de erro
					} else
						listaImprime(listaTokens);
						listaTokens = listaLibera(listaTokens);
					break;

				case ' ':
					index = 0;
					if(strlen(token) > 0) {
						printf(">%s<\n", token);
						listaTokens = listaInsere(token, listaTokens);
						listaImprime(listaTokens);
					}
					token[index] = '\0';
					break;

				default:
					token[index++] = charLido;
					token[index] = '\0';
					break;
			}
		}
		
		return 1;
	}
	
	return 0;
}