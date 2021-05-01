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
	}
	else
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

int listaTamanho(Token* lista)
{
	Token* P = lista;
	int contador = 0;
	while(P != NULL) {
		contador++;
		P = P->proximo;
	}
	return contador;
}

char** listaGetTokenArray(Token* lista)
{
	if(lista != NULL) {
		int tamanhoLista = listaTamanho(lista);
		char** arrayArgumentos = (char**) malloc((tamanhoLista + 1) * sizeof(char*));
		Token* iterator = lista;
		for(int i = 0; i < tamanhoLista; i++) {
			arrayArgumentos[i] = iterator->texto;
			iterator = iterator->proximo;
		}
		arrayArgumentos[tamanhoLista] = NULL;
		return arrayArgumentos;
	}
	else
		return NULL;
}

char* listaGetByIndex(int index, Token* lista)
{
	Token* P = lista;
	for(int i = 0; i < index; i++) {
		if(P == NULL) return NULL;
		P = P->proximo;
	}
	return P->texto;
}

int listaIsEmpty(Token* lista) {
	return lista == NULL ? 1 : 0;
}

void listaImprime(Token* lista)
{
	Token* P = lista;
	while(P != NULL) {
		// printf ("texto: >%s<\n", P->texto);
		printf (" >%s< ", P->texto);
		P = P->proximo;
	}
}

Token* listaRemover(char* item, Token* lista)
{
	Token* P = lista, *temp = lista;

	while(P != NULL) {
		if(strcmp(P->texto, item) == 0) {
			if(P == lista)
				lista = P->proximo;
			else
				temp->proximo = P->proximo;
			free(P->texto);
			free(P);
			break;
		}
		temp = P;
		P = P->proximo;
	}

	return lista;
}