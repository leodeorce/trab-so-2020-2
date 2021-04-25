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

void armageddon(void)
{

}

void liberamoita(void)
{
	
}

void catchUSR(int num)
{
	FILE* fd = fopen("./jacare.txt", "r");
	char c;
	while((c = fgetc(fd)) != EOF) printf("%c", c);
	fclose(fd);
}

int main(void)
{
	// system("clear");
	char   token[MAX_COMMAND_LENGTH];
	char   charLido = '\n';
	int    index = 0;
	int    loop = 1;
	int    background = 0;
	int    qtdeComandos = 0;
	Token* listaTokens = listaInicializa();

	struct sigaction act;
	act.sa_handler = catchUSR;
	act.sa_flags = 0;

	sigset_t newsigset;
	if(sigemptyset(&newsigset) == -1
	   || sigaddset(&newsigset, SIGINT)
	   || sigaddset(&newsigset, SIGQUIT)
	   || sigaddset(&newsigset, SIGTSTP)) {
		   perror("Falha ao criar máscara de sinais");
		   exit(1);
	   }
	
	act.sa_mask = newsigset;

	if(sigaction(SIGUSR1, &act, NULL) == -1) {
		perror("Falha ao associar tratador com SIGUSR1");
		exit(1);
	}
	if(sigaction(SIGUSR2, &act, NULL) == -1) {
		perror("Falha ao associar tratador com SIGUSR2");
		return 1;
	}

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
					int tamanhoLista = listaTamanho(listaTokens);
					if(background == 0 && tamanhoLista > 0) {
						if(tamanhoLista == 1) {
							if(strcmp(listaGetByIndex(0, listaTokens), "armageddon") == 0) {
								armageddon();
								loop = 0;
								break;
							}
							else if(strcmp(listaGetByIndex(0, listaTokens), "liberamoita") == 0) {
								liberamoita();
								loop = 0;
								break;
							}
						}
						if(tamanhoLista <= 4)
							executarForeground(listaTokens);
						else {
							printf("Limite de argumentos para o comando '%s' excedido\n",
								listaGetByIndex(0, listaTokens));
							loop = 0;
						}
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

		listaTokens = listaLibera(listaTokens);
		token[0] = '\0';
		// return 0;
	}

	return 0;
}