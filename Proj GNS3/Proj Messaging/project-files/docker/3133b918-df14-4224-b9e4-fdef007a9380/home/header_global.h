/*  AUTORES: Dário Félix N.2018275530 | Vadilson Jacob N.2015270539  */

// FICHEIRO: header_global.h (para global.c)



#ifndef PROJAUX_HEADER_GLOBAL_H
#define PROJAUX_HEADER_GLOBAL_H



/* BIBLIOTECAS */
#include "header_comum.h"



/* CONSTANTES */



/* MACROS */



/* DEFINIÇÕES e ESTRUTURAS */



/* VARIAVEIS GLOBAIS */



/* VARIAVEIS GLOBAIS - EXTERNO */
extern char quemSouEu[SIZE_STR_QUEM];



/* PROTOTIPOS */
void printInfo (const char * quem, const char * msg);
void printErro (const char * quem, const char * onde, const char * msg);
void printAviso (const char * quem, const char * onde, const char * msg);
type_resultado getListaChaveValorFromPacote (const char pacote[SIZE_STR_PACOTE], ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]);
type_resultado insertChaveValorIntoPacote (char pacote[SIZE_STR_PACOTE], const char chave[SIZE_STR_CHAVE], const char valor[SIZE_STR_VALOR]);
char * getValorByChaveFromLista (const char chave[SIZE_STR_CHAVE], ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]);
type_resultado inicializarPacote (char pacote[SIZE_STR_PACOTE]);
struct sockaddr_in * mallocEndereco(void);
void getStringStdin(char string[], int n);
type_boolean verificaAndCorrigeMsg (char * msg, int tamanho);
type_boolean verificaMsg (const char * msg, int tamanho);
type_resultado getPacoteFromListaChaveValor (char pacote[SIZE_STR_PACOTE], const ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]);
void intro(void);



#endif //PROJAUX_HEADER_GLOBAL_H
