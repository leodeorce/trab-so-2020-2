#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "lista.h"

#define MAX_COMMAND_LENGTH 2097152

void executarForeground(Token* listaTokens)
{
	char** arrayArgumentos = listaGetTokenArray(listaTokens);
	int wstatus;
	pid_t pid = fork();

	if(pid == 0) {
		signal(SIGUSR1, SIG_IGN);
		signal(SIGUSR2, SIG_IGN);
		execvp(arrayArgumentos[0], arrayArgumentos);
	}
	else if(pid > 0) {
		wait(&wstatus);
		if (WIFEXITED(wstatus) > 0)
			printf("Filho retornou com código %d\n", WEXITSTATUS(wstatus));
		if (WIFSIGNALED(wstatus) > 0)
			printf("Filho terminou ao receber o sinal %d\n", WTERMSIG(wstatus));
	}
	else
		perror("Falha ao executar fork()");
}

int main(void)
{
	// system("clear");
	char   token[MAX_COMMAND_LENGTH];
	char   charLido;
	int    index = 0;
	int    loop = 1;
	int    background = 0;
	int    qtdeComandos = 0;
	Token* listaTokens = listaInicializa();

	while(1) {

		printf("vsh> ");
		index = 0;
		loop = 1;
		background = 0;
		qtdeComandos = 0;

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
					if(background == 0 && listaIsEmpty(listaTokens) != 1) {
						executarForeground(listaTokens);
					}
					loop = 0;
					break;

				case '|':
					if(strlen(token) > 0) {
						printf("Símbolo '|' inesperado\n");
						loop = 0;
					}
					else {
						background = 1;
						qtdeComandos++;
						if(qtdeComandos >= 5) {
							printf("Favor inserir no máximo 5 comandos\n");
							loop = 0;
						}
						listaImprime(listaTokens);
						listaTokens = listaLibera(listaTokens);
					}
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
		
		return 0;
	}

	return 0;
}