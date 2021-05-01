/**
 * Lista encadeada simples
 */

#ifndef LISTA_H_
#define LISTA_H_

/**
 * Estrutura para armazenar o texto de um token
 */
typedef struct token Token;

/**
 * Inicializa uma lista
 * @param void
 * @return Ponteiro para o início da lista encadeada
 */
Token* listaInicializa(void);

/**
 * Insere um aluno na primeira posição da lista de alunos
 * @param token Token a ser inserido na lista
 * @param lista Lista na qual o token deve ser inserido
 * @return Ponteiro para o início da lista encadeada
 */
Token* listaInsere(char* texto, Token* lista);

/**
 * Libera toda a memória alocada para a lista
 * @param lista A lista encadeada de tokens
 * @return Ponteiro para a lista encadeada liberada
 */
Token* listaLibera(Token* lista);

/**
 * Verifica a quantidade de elementos na lista
 * @param lista A lista encadeada de tokens
 * @return Quantidade de elementos na lista encadeada
 */
int listaTamanho(Token* lista);

/**
 * Cria uma array alocado dinamicamente com os nomes dos tokens da lista
 * @param lista A lista encadeada de tokens
 * @return Ponteiro para o array criado
 */
char** listaGetTokenArray(Token* lista);

/**
 * Usa um índice para buscar um item da lista
 * @param index Índice do item desejado
 * @param lista A lista encadeada de tokens
 * @return Ponteiro para o texto do token desejado
 */
char* listaGetByIndex(int index, Token* lista);

/**
 * Verifica se a lista encadead está vazia
 * @param lista A lista encadeada de tokens
 * @return 1 caso vazia e 0 caso contrário
 */
int listaIsEmpty(Token* lista);

void listaImprime(Token* lista);

Token* listaRemover(char* item, Token* lista);

#endif /* LISTA_H_ */
