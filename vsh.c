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
	// Cria array com os elementos a serem passados para execvp()
	char** arrayArgumentos = listaGetTokenArray(listaTokens);
	int wstatus;
	pid_t pid = fork();

	if(pid == 0) {  // Caso filho
		signal(SIGUSR1, SIG_IGN);
		signal(SIGUSR2, SIG_IGN);
		execvp(arrayArgumentos[0], arrayArgumentos);
	}
	else if(pid > 0) {  // Caso pai
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
	while((c = fgetc(fd)) != EOF) printf("%c", c);  // Imprime cada caractere do arquivo
	fclose(fd);
}

int main(void)
{
	system("clear");          // Limpa a tela do terminal atual
	char   token[MAX_COMMAND_LENGTH];  // Armazena comandos ou argumentos
	char   charLido = '\n';   // Armazena um caractere lido de stdin
	int    index = 0;         // Indica a posição em 'token' na qual se deve escrever
	int    loop = 1;          // Indica se é hora de parar leitura e mostrar o prompt
	int    background = 0;    // Indica se um operador especial de pipe foi lido
	int    qtdeComandos = 0;  // Quantidade de comandos lidos até o momento

	/** A lista é preenchida com 'tokens' referentes ao comando passado por stdin.
	 *  O primeiro elemento da lista é sempre o nome do comando e os próximos são
	 *  seus argumentos. A lista é esvaziada sempre que um operador especial '|' é
	 *  detectado ou o final da linha é alcançado. */
	Token* listaTokens = listaInicializa();

	// Inicializa nova estrutura sigaction com o handler 'catchUSR'
	struct sigaction act;
	act.sa_handler = catchUSR;
	act.sa_flags = 0;

	// Cria máscara de sinais a serem bloqueados no tratamento de SIGUSR1/SIGUSR2
	sigset_t newsigset;
	if(sigemptyset(&newsigset) == -1
	   || sigaddset(&newsigset, SIGINT)
	   || sigaddset(&newsigset, SIGQUIT)
	   || sigaddset(&newsigset, SIGTSTP)) {
		   perror("Falha ao criar máscara de sinais");
		   exit(1);
	   }
	
	act.sa_mask = newsigset;  // Insere nova máscara na estrutura

	// Associa os sinais a serem tratados à estrutura
	if(sigaction(SIGUSR1, &act, NULL) == -1) {
		perror("Falha ao associar tratador com SIGUSR1");
		exit(1);
	}
	if(sigaction(SIGUSR2, &act, NULL) == -1) {
		perror("Falha ao associar tratador com SIGUSR2");
		return 1;
	}
	
	while(1) {  // Loop do prompt

		printf("vsh> ");
		index = 0;          // 'index' em 0 indica que token é reescrito
		loop = 1;           // 'loop' em 1 indica que novos caracteres serão lidos
		background = 0;     // 'background' em 0 indica que ainda não lemos um '|'
		qtdeComandos = 0;   // 'qtdeComandos' em 0 indica que ainda não lemos comandos

		while(loop == 1) {  // Loop de leitura de caracteres

			scanf("%c", &charLido);

			switch(charLido) {

				// Mesmo código para '\n' e '\r'
				case '\n':
				case '\r':
					if(strlen(token) > 0) {  /* Se existirem espaços antes da
                                                quebra de linha, 'token' estará vazio. */
						printf(">%s<\n", token);  // Debug
						listaTokens = listaInsere(token, listaTokens);
						listaImprime(listaTokens);  // Debug
					}
					int tamanhoLista = listaTamanho(listaTokens);
					// Verifica se o comando a executar é foreground ou background
					// Verifica se existe comando a executar
					if(background == 0 && tamanhoLista > 0) {
						if(tamanhoLista == 1) {  /* Se existe apenas um token na lista,
                                                    há chance que este é operação interna. */
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
						if(tamanhoLista <= 4)  /* Se a quantidade de tokens é maior que 4,
                                                  a quantidade de argumentos é maior que 3. */
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
					if(strlen(token) > 0) {  /* Se o token não está vazio, então o operador
                                                especial foi encontrado no meio de uma palavra */
						printf("Símbolo '|' inesperado\n");
						loop = 0;
					}
					else {
						background = 1;  // As próximas execuções devem ser em background
						qtdeComandos++;
						if(qtdeComandos >= 5) {
							printf("Favor inserir no máximo 5 comandos\n");
							loop = 0;
						}
						listaImprime(listaTokens);  // Debug

						// Provavelmente aqui é necessário guardar múltiplas listas

						listaTokens = listaLibera(listaTokens);
					}
					break;

				case ' ':
					index = 0;  // Um token foi finalizado
					if(strlen(token) > 0) {  /* Se o token finalizado não é vazio
                                                então ele deve ser inserido na lista */
						printf(">%s<\n", token);  // Debug
						listaTokens = listaInsere(token, listaTokens);
						listaImprime(listaTokens);  // Debug
					}
					token[index] = '\0';  // Reinicia o token
					break;

				default:
					token[index++] = charLido;
					token[index] = '\0';
					break;
			}
		}

		listaTokens = listaLibera(listaTokens);
		token[0] = '\0';  // Caso um sinal seja recebido e o token não é reiniciado
		// return 0;
	}

	return 0;
}