#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "lista.h"

#define MAX_COMMAND_LENGTH 1000

#define LEITURA 0
#define ESCRITA 1

#define closePipes()        \
	close(fd[0][LEITURA]);  \
	close(fd[0][ESCRITA]);  \
	close(fd[1][LEITURA]);  \
	close(fd[1][ESCRITA]);  \
	close(fd[2][LEITURA]);  \
	close(fd[2][ESCRITA]);  \
	close(fd[3][LEITURA]);  \
	close(fd[3][ESCRITA])

/**
 *  Executa um comando em foreground
 *  @param listaTokens Lista de tokens de leitura da entrada
 */
void executarForeground(Token* listaTokens)
{
	// Garante que sinais por teclado afetem apenas o processo filho
	signal(SIGTSTP, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	// Cria array com os elementos a serem passados para execvp()
	char** arrayArgumentos = listaGetTokenArray(listaTokens);
	int wstatus;
	pid_t pid = fork();

	if(pid == 0) {  // Caso filho
		signal(SIGTSTP, SIG_DFL);
		signal(SIGINT,  SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGUSR1, SIG_IGN);  // Vacinado
		signal(SIGUSR2, SIG_IGN);  // Vacinado
		execvp(arrayArgumentos[0], arrayArgumentos);
	}
	else if(pid > 0) {  // Caso pai
		waitpid(pid, &wstatus, WUNTRACED);
		free(arrayArgumentos);
		if (WIFEXITED(wstatus) > 0) {
			if(WEXITSTATUS(wstatus) != 0)  // Retorno 0 é implícito
				printf("Filho retornou com código %d\n", WEXITSTATUS(wstatus));
		}
		if (WIFSIGNALED(wstatus) > 0)
			printf("Filho terminou ao receber o sinal %d\n", WTERMSIG(wstatus));
	}
	else  // pid < 0
		perror("Falha ao executar fork()");

	signal(SIGTSTP, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT,  SIG_DFL);
}

/**
 *  Executa os comandos em background
 *  @param grupoBackground Array de listas de comandos
 *  @param listaSID Lista de SIDs de sessões background criadas pela shell
 *  @param indexListas Posição da array com a última lista
 *  @return Lista de SIDs modificada
 */
Token* executarBackground(Token** grupoBackground, Token* listaSID, int indexListas)
{
	char sid[10];  // Armazena temporariamente o SID criado por um processo intermediário
	char** arrayArgumentos; // Armazena uma array de argumentos a ser enviada por 'execvp'
	pid_t pid = fork();

	if(pid == 0){  // Caso processo intermediário para criar a nova sessão
		
		// Processos intermediarios terminando com os SIGUSR's
		signal(SIGUSR1, SIG_DFL);
		signal(SIGUSR2, SIG_DFL);
		
		pid_t pid[5];
		int   wstatus;
		int   sigusr = 0;  // Indica se um processo filho terminou por SIGUSR1/2
		int   fd[4][2];    // Descritores para no máximo 4 pipes

		setsid();  // Processo intermediário cria a própria sessão
		
		if(pipe(fd[0]) == -1){ fprintf(stderr, "Erro ao criar o pipe\n"); _exit(1); }
		if(pipe(fd[1]) == -1){ fprintf(stderr, "Erro ao criar o pipe\n"); _exit(1); }
		if(pipe(fd[2]) == -1){ fprintf(stderr, "Erro ao criar o pipe\n"); _exit(1); }
		if(pipe(fd[3]) == -1){ fprintf(stderr, "Erro ao criar o pipe\n"); _exit(1); }
		
		for(int i = 0; i <= indexListas; i++) {

			arrayArgumentos = listaGetTokenArray(grupoBackground[i]);
			pid[i] = fork();

			if(pid[i] == 0) {
				
				// Todos leem de um pipe exceto o primeiro processo
				switch(i) {
					case 1: dup2(fd[0][LEITURA], 0); break;
					case 2: dup2(fd[1][LEITURA], 0); break;
					case 3: dup2(fd[2][LEITURA], 0); break;
					case 4: dup2(fd[3][LEITURA], 0); break;
				}

				// Todos escrevem em um pipe exceto o último processo
				if(i != indexListas)
					dup2(fd[i][ESCRITA], 1);
				
				closePipes();

				// O primeiro elemento é sempre o nome do comando
				execvp(arrayArgumentos[0], arrayArgumentos);
			}

			free(arrayArgumentos);
		}
		
		closePipes();
		
		while(1){  // Loop para prender o intermediário
			// Liberando o intermediário caso não tenha mais processo para terminar
			if((waitpid(-1, &wstatus, WNOHANG) == -1) && (errno == ECHILD)){
				break;
			}
			if(WIFSIGNALED(wstatus) > 0){
				// Liberando o intermediário caso algum processo filho tenha
				// recebido SIGUSR1 ou SIGUSR2
				if(WTERMSIG(wstatus) == SIGUSR1 || WTERMSIG(wstatus) == SIGUSR2){
					sigusr = 1;
					break;
				}
			}
		}

		int i = 0;
        // Libera todas as listas auxiliares para processos background
			// pois este processo é um fork da shell
        while(i <= indexListas && grupoBackground[i] != NULL) {
            grupoBackground[i] = listaLibera(grupoBackground[i]);
            i++;
        }

		listaSID = listaLibera(listaSID);

		if(sigusr){  // Terminando a sessão caso houver ocorrência do SIGUSR1 ou do SIGUSR2
			kill(-getpgid(getpid()), SIGKILL);
		}
		
		exit(0);
	}
	else if(pid > 0){  // Caso pai
		// Transforma PID do processo intermediário (= SID) em string
		sprintf(sid, "%d", pid);
		listaSID = listaInsere(sid, listaSID);
	}
	else
		perror("Falha ao executar fork()");

	return listaSID;
}

/**
 *  Termina os descendentes
 *  @param listaSID Lista de SIDs de sessões background criadas pela shell
 *  @return Lista de SIDs vazia
 */
Token* armageddon(Token* listaSID)
{
	int numProcessos = listaTamanho(listaSID);
	for(int i = 0; i < numProcessos; i++) {
		// Transforma o SID armazenado em texto para pid_t
		// SID = PGID nos processos criados em 'executarBackground'
		// Envia SIGKILL para todos os processos da sessão
		kill( -((pid_t) atoi(listaGetByIndex(i, listaSID))), SIGKILL );
	}
	listaSID = listaLibera(listaSID);
	kill(0, SIGINT);  // Caso algum processo foreground foi pausado e continuado
	return listaSID;
}

/**
 *  Libera descendentes zumbis
 *  @param listaSID Lista de SIDs de sessões background criadas pela shell
 *  @return Lista de SIDs modificada
 */
Token* liberamoita(Token* listaSID)
{
	int numProcessos = listaTamanho(listaSID);
	int wstatus;
	int i = 0;

	// 'wait' em cada processo intermediário
	while(i < numProcessos) {

		char temp[10];
		pid_t pid = waitpid(-1, &wstatus, WNOHANG); // Resgata zombie

		// Remove o SID da lista caso zombie resgatado
		if(pid > 0 && WIFEXITED(wstatus) > 0){
			sprintf(temp, "%d", pid);
			listaSID = listaRemover(temp, listaSID);
			i--;
			numProcessos--;
		}

		i++;
	}

	i = 0;
	numProcessos = listaTamanho(listaSID);

	// Removendo da lista de SIDs itens inválidos
	while(i < numProcessos) {

		char temp[10];
		pid_t pid = (pid_t) atoi(listaGetByIndex(i, listaSID));

		// waitpid retorna -1 em caso de erro e errno verifica se isso se deve
			// à ausência de um filho com o pid buscado
		if((waitpid(pid, NULL, WNOHANG)) == -1 && errno == ECHILD){
			sprintf(temp, "%d", pid);
			listaSID = listaRemover(temp, listaSID);
			i--;
			numProcessos--;
		}

		i++;
	}

	numProcessos = listaTamanho(listaSID);

	return listaSID;
}

/**
 *  Handler para os sinais SIGUSR1 e SIGUSR2 na shell
 */
void catchUSR(int num)
{
	FILE* fd = fopen("./jacare.txt", "r");
	char c;
	while((c = fgetc(fd)) != EOF) printf("%c", c);  // Imprime cada caractere do arquivo
	fclose(fd);
}

/**
 *  Leitura do restante de uma entrada em stdin caso erro seja encontrado
 */
void resetarEntrada(void) {
	char c = '\0';
	while(c != '\n') scanf("%c", &c);
}

int main(void)
{
	system("clear -x");       // Limpa a tela do terminal atual
	char   token[MAX_COMMAND_LENGTH];  // Armazena comandos ou argumentos
	char   charLido = '\n';   // Armazena um caractere lido de stdin
	int    finalizar = 0;	  // Finaliza a shell
	int    indexToken = 0;    // Indica a posição em 'token' na qual se deve escrever
	int    loop = 1;          // Indica se é hora de parar leitura
	int    background = 0;    // Indica se um operador especial de pipe foi lido
	int    indexListas = 0;   // Índice no qual inserir uma lista em 'grupoBackground'

	/** A lista é preenchida com 'tokens' referentes ao comando passado por stdin.
	 *  O primeiro elemento da lista é sempre o nome do comando e os próximos são
	 *  seus argumentos. A lista é esvaziada sempre que um operador especial '|' é
	 *  detectado ou o final da linha é alcançado. */
	Token* listaTokens = listaInicializa();

	// Armazena os SIDs de processos criados em background
	Token* listaSID = listaInicializa();

	// Armazena listas de comandos a serem executados em background
	Token* grupoBackground[5] = { NULL };

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
		token[0] = '\0';    // Resetando token
		indexToken = 0;     // 'indexToken' em 0 indica que token é reescrito
		loop = 1;           // 'loop' em 1 indica que novos caracteres serão lidos
		background = 0;     // 'background' em 0 indica que ainda não lemos um '|'
		indexListas = 0;    /* 'indexListas' em 0 indica que ainda não terminamos
                               de ler o primeiro comando */

		while(loop == 1) {  // Loop de leitura de caracteres

			scanf("%c", &charLido);

			switch(charLido) {

				case '\n':

					if(strlen(token) > 0) {  /* Se existirem espaços antes da
                                                quebra de linha, 'token' estará vazio. */
						listaTokens = listaInsere(token, listaTokens);
					}

					if(listaIsEmpty(listaTokens) == 1 && background == 1) {
						printf("Erro: não há comando após último operador '|'\n");
						loop = 0;
						break;
					}

					int tamanhoLista = listaTamanho(listaTokens);
					
					// Verifica se o comando a executar é foreground ou background
					// Verifica se existe comando a executar
					if(background == 0 && tamanhoLista > 0) {

						if(tamanhoLista == 1) {  /* Se existe apenas um token na lista,
                                                    há chance que este é operação interna. */
							if(strcmp(listaGetByIndex(0, listaTokens), "armageddon") == 0) {
								listaSID = armageddon(listaSID);
								loop = 0;
								finalizar = 1;
								break;
							}
							else if(strcmp(listaGetByIndex(0, listaTokens), "liberamoita") == 0) {
								listaSID = liberamoita(listaSID);
								loop = 0;
								break;
							}
						}

						if(tamanhoLista <= 4) {  /* Se a quantidade de tokens é maior que 4,
                                                    a quantidade de argumentos é maior que 3. */
							executarForeground(listaTokens);

						} else {
							printf("Erro: limite de argumentos para o comando '%s' excedido\n",
								listaGetByIndex(0, listaTokens));
							loop = 0;
							break;
						}
					}

					else if(background == 1) {

						if(tamanhoLista >= 5) {  /* Se a quantidade de tokens é maior que 4,
                                                    a quantidade de argumentos é maior que 3. */
							printf("Erro: limite de argumentos para o comando '%s' excedido\n",
								listaGetByIndex(0, listaTokens));
							loop = 0;
							break;
						}

						// Adiciona a última lista no grupo de comandos em background
						grupoBackground[indexListas] = listaTokens;
						listaSID = executarBackground(grupoBackground, listaSID, indexListas);
					}

					loop = 0;
					break;

				case '|':

					if(strlen(token) > 0) {  /* Se o token não está vazio, então o operador
                                                especial foi encontrado no meio de uma palavra */
						printf("Erro: símbolo '|' inesperado\n");
						resetarEntrada();
						loop = 0;
						break;
					}

					if(indexListas >= 4) {
						printf("Erro: favor inserir no máximo 5 comandos\n");
						resetarEntrada();
						loop = 0;
						break;
					}

					if(listaTamanho(listaTokens) >= 5) {
						printf("Erro: limite de argumentos para o comando '%s' excedido\n",
								listaGetByIndex(0, listaTokens));
						resetarEntrada();
						loop = 0;
						break;
					}

					background = 1;  // As execuções devem ser em background
					grupoBackground[indexListas] = listaTokens;
					indexListas++;
					listaTokens = listaInicializa();
					break;

				case ' ':

					indexToken = 0;  // Um token foi finalizado

					if(strlen(token) > 0) {  /* Se o token finalizado não é vazio
                                                então ele deve ser inserido na lista */
						listaTokens = listaInsere(token, listaTokens);
					}

					token[indexToken] = '\0';  // Reinicia o token
					break;

				default:
					token[indexToken++] = charLido;
					token[indexToken] = '\0';
					break;
			}
		}

		if(background == 1) {
			int i = 0;
			// Libera todas as listas auxiliares para processos background
			while(i < indexListas && grupoBackground[i] != NULL) {
				grupoBackground[i] = listaLibera(grupoBackground[i]);
				i++;
			}
		}
		
		if(listaIsEmpty(listaTokens) == 0)
			listaTokens = listaLibera(listaTokens);  // Caso saída do loop por erro
		else if(background == 1)
			grupoBackground[indexListas] = listaLibera(grupoBackground[indexListas]);

		listaTokens = listaInicializa();  // Reinicia a lista auxiliar

		if(finalizar == 1)	// Saindo do loop principal da shell
			break;
	}

	return 0;
}