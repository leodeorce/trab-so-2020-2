#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lista.h"

struct token {
	char*  texto;
	Token* proximo;
};

Token* listaInicializa(void)
{
	return NULL;
}

Token* listaInsere(char* texto, Token* lista)
{
	Token* novoToken = (Token*) malloc(sizeof(Token));
	char* textoDinamico = (char*) malloc(strlen(texto) + 1);
	strcpy(textoDinamico, texto);
	novoToken->texto = textoDinamico;
	novoToken->proximo = NULL;
	if(lista != NULL) {
		Token* iterator = lista;
		while(iterator->proximo != NULL)
			iterator = iterator->proximo;
		iterator->proximo = novoToken;
	} else
		lista = novoToken;
	return lista;
}

Token* listaLibera(Token* lista)
{
	Token* P = lista;
	Token* temp;
	while(P != NULL) {
		temp = P->proximo;
		free(P->texto);
		free(P);
		P = temp;
	}
	return P;
}

void listaImprime(Token* lista)
{
	Token* P = lista;
	while(P != NULL) {
		printf ("texto: >%s<\n", P->texto);
		P = P->proximo;
	}
}