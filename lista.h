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

void listaImprime(Token* lista);

#endif /* LISTA_H_ */
