/*  AUTORES: Dário Félix N.2018275530 | Vadilson Jacob N.2015270539  */

// FICHEIRO: global.c (contem funcoes comuns, tanto para o lado do cliente como do servidor)



/* HEADERS */
#include "header_global.h"



/* FUNCOES */
void printErro (const char * quem, const char * onde, const char * msg) {
    if (quem == NULL)
        quem = "?";
    if (onde == NULL)
        onde = "?";
    if (msg == NULL)
        msg = "?";

    printf("[%s]  ### ERRO ###  %s(): \"%s\"\n", quem, onde, msg);
    fflush(stdout);
}



void printAviso (const char * quem, const char * onde, const char * msg) {
    if (quem == NULL)
        quem = "?";
    if (onde == NULL)
        onde = "?";
    if (msg == NULL)
        msg = "?";

    printf("[%s]  ### AVISO ###  %s(): \"%s\"\n", quem, onde, msg);
    fflush(stdout);
}



void printInfo (const char * quem, const char * msg) {
    if (quem == NULL)
        quem = "?";
    if (msg == NULL)
        msg = "?";

    printf("[%s]  %s\n", quem, msg);
    fflush(stdout);
}



type_resultado getListaChaveValorFromPacote (const char pacote[SIZE_STR_PACOTE], ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]) {
    int i, max;
    char * token;
    char listaBruto [NUM_MAX_PARES_ARRAY][SIZE_STR_CHAVE + SIZE_STR_VALOR];
    char pacoteTemp [SIZE_STR_PACOTE];

    if (listaChaveValor == NULL || pacote == NULL) {
        printErro(quemSouEu, "getListaChaveValorFromPacote", "params == NULL");
        return ERRO;
    }

    snprintf(pacoteTemp, SIZE_STR_PACOTE, "%s", pacote);

    token = strtok(pacoteTemp, PROTOCOLO_SEPARADOR_PARES);
    for(i=0 ; token != NULL && i < NUM_MAX_PARES_ARRAY ; i++) {
        if (snprintf(listaBruto[i], SIZE_STR_CHAVE + SIZE_STR_VALOR, "%s", token) < 0){
            printErro(quemSouEu, "getListaChaveValorFromPacote", "listaBruto: snprintf() < 0");
            return ERRO;
        }
        token = strtok(NULL, PROTOCOLO_SEPARADOR_PARES);
    }

    max = i;

    if (token != NULL || i == 0) {
        printErro(quemSouEu, "getListaChaveValorFromPacote", "token != NULL || i == 0");
        return ERRO;
    }

    for(i=0 ; i < max ; i++) {
        if ((token = strtok(listaBruto[i], PROTOCOLO_SEPARADOR_CHAVE_VALOR)) == NULL) {
            printErro(quemSouEu, "getListaChaveValorFromPacote", "chave: token == NULL");
            return ERRO;
        }

        if (snprintf(listaChaveValor[i].chave, SIZE_STR_CHAVE, "%s", token) < 0){
            printErro(quemSouEu, "getListaChaveValorFromPacote", "listaChave: snprintf() < 0");
            return ERRO;
        }

        if ((token = strtok(NULL, PROTOCOLO_SEPARADOR_CHAVE_VALOR)) == NULL) {
            printErro(quemSouEu, "getListaChaveValorFromPacote", "valor: token == NULL");
            return ERRO;
        }

        if (snprintf(listaChaveValor[i].valor, SIZE_STR_VALOR, "%s", token) < 0){
            printErro(quemSouEu, "getListaChaveValorFromPacote", "listaValor: snprintf() < 0");
            return ERRO;
        }
    }

    for(i=max ; i < NUM_MAX_PARES_ARRAY ; i++) {
        listaChaveValor[i].chave[0] = '\0';
        listaChaveValor[i].valor[0] = '\0';
    }

    if (getValorByChaveFromLista(CHAVE_TYPE, listaChaveValor) == NULL) {
        printErro(quemSouEu, "getListaChaveValorFromPacote", "CHAVE_TYPE em falta");
        return ERRO;
    }

    return SUCESSO;
}



type_resultado getPacoteFromListaChaveValor (char pacote[SIZE_STR_PACOTE], const ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]) {
    int i;

    if (listaChaveValor == NULL || pacote == NULL) {
        printErro(quemSouEu, "getPacoteFromListaChaveValor", "params == NULL");
        return ERRO;
    }

    if (inicializarPacote(pacote) == ERRO) {
        printErro(quemSouEu, "getPacoteFromListaChaveValor", "nao foi possivel inicializar o pacote");
        return ERRO;
    }

    for (i=0 ; i < NUM_MAX_PARES_ARRAY &&  listaChaveValor[i].chave[0] != '\0'; i++) {
        if(insertChaveValorIntoPacote(pacote, listaChaveValor[i].chave, listaChaveValor[i].valor) == ERRO) {
            printErro(quemSouEu, "getPacoteFromListaChaveValor", "nao foi possivel inserir ChaveValor no pacote");
            return ERRO;
        }
    }

    return SUCESSO;
}



type_resultado insertChaveValorIntoPacote (char pacote[SIZE_STR_PACOTE], const char chave[SIZE_STR_CHAVE], const char valor[SIZE_STR_VALOR]) {
    if (chave == NULL || valor == NULL || pacote == NULL) {
        printErro(quemSouEu, "insertChaveValorIntoPacote", "params == NULL");
        return ERRO;
    }

    char pacoteTemp [SIZE_STR_PACOTE];
    snprintf(pacoteTemp, SIZE_STR_PACOTE, "%s", pacote);

    if (snprintf(pacote, SIZE_STR_PACOTE, "%s%s%s%s%s", pacoteTemp, PROTOCOLO_SEPARADOR_PARES, chave, PROTOCOLO_SEPARADOR_CHAVE_VALOR, valor) < 0){
        printErro(quemSouEu, "insertChaveValorIntoPacote", "snprintf() < 0");
        return ERRO;
    }

    return SUCESSO;
}



char * getValorByChaveFromLista (const char chave[SIZE_STR_CHAVE], ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]) {
    int i;

    if (chave != NULL && listaChaveValor != NULL) {
        for (i = 0; i < NUM_MAX_PARES_ARRAY && listaChaveValor[i].chave[0] != '\0'; i++) {
            if (strcmp(chave, listaChaveValor[i].chave) == 0) {
                return listaChaveValor[i].valor;
            }
        }
    }

    printErro(quemSouEu, "getValorByChaveFromLista", "return NULL");

    return NULL;
}



type_resultado inicializarPacote (char pacote[SIZE_STR_PACOTE]) {
    if (pacote == NULL) {
        printErro(quemSouEu, "inicializarPacote", "pacote == NULL");
        return ERRO;
    }

    pacote[0] = '\0';

    return SUCESSO;
}



struct sockaddr_in * mallocEndereco(void) {
    struct sockaddr_in * enderecoTemp = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));

    if (enderecoTemp == NULL) {
        printErro(quemSouEu, "mallocEndereco", "endereco == NULL");
    } else {
        bzero((void *) enderecoTemp, sizeof(struct sockaddr_in));
    }

    return enderecoTemp;
}



/**
* Funcao segura para ler, guardar e tratar dados do STDIN, limpando o restante.
* @param string char[] - local onde é guardada os chars recolhidos
* @param n int - tamanho máximo suportado por string
* @return void
* @author Dario Felix
* @version 1.0
*/
void getStringStdin(char string[], int n) {
    int i = 0, c;

    if (string == NULL) {
        printErro(quemSouEu, "getStringStdin", "params == NULL");
        return;    // erro
    }

    // ler stdin ate atingir o final do stdin ou da memoria disponivel
    while (i < (n - 1) && ((string[i] = (char)getchar()) != '\n' && string[i] != EOF))
        i++;

    // (If) Sobrou dados no stdin > Limpar stdin
    if (string[i] != '\n' && string[i] != EOF)
        while ((c = getchar()) != '\n' && c != EOF);

    // Colocar terminador '\0' no fim
    string[i] = '\0';
}



type_boolean verificaAndCorrigeMsg (char * msg, int tamanho) {
    int i;
    type_boolean fator;

    if (msg == NULL) {
        printErro(quemSouEu, "verificaAndCorrigeMsg", "params == NULL");
        return TRUE;    // erro
    }

    for (fator = FALSE, i=0 ; msg[i] != '\0' && i < tamanho ; i++) {
        if (msg[i] == PROTOCOLO_SEPARADOR_CHAVE_VALOR_CHAR || msg[i] == PROTOCOLO_SEPARADOR_PARES_CHAR) {
            msg[i] = ' ';
            fator = TRUE;   // msg invalida (erro)
        }
    }

    return fator;
}



type_boolean verificaMsg (const char * msg, int tamanho) {
    int i;

    if (msg == NULL) {
        printErro(quemSouEu, "verificaMsg", "params == NULL");
        return TRUE;    // erro
    }

    for (i=0 ; msg[i] != '\0' && i < tamanho ; i++) {
        if (msg[i] == PROTOCOLO_SEPARADOR_CHAVE_VALOR_CHAR || msg[i] == PROTOCOLO_SEPARADOR_PARES_CHAR) {
            return TRUE;   // msg invalida (erro)
        }
    }

    return FALSE;
}



void intro(void) {
    printf("\n********************************************************************\n");
    printf("* PROJETO: Sistema de messaging | FCTUC/LEI - Redes de Comunicacao *\n");
    printf("* AUTORES: Dario Felix N.2018275530 | Vadilson Jacob N.2015270539  *\n");
    printf("********************************************************************\n\n\n");

    fflush(stdout);
}


