/*  AUTORES: Dário Félix N.2018275530 | Vadilson Jacob N.2015270539  */

// FICHEIRO: header_comum.h (comum ao cliente.c, servidor.c, global.c)



#ifndef PROJAUX_HEADER_COMUM_H
#define PROJAUX_HEADER_COMUM_H



/* PRE-BIBLIOTECAS */
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE       // sacrilegious
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE         // sacrilegious
#endif



/* BIBLIOTECAS */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <strings.h>
#include <netdb.h>
#include <sys/stat.h>
#include <netinet/in.h>



/* CONSTANTES */
// generico
#define QUEM_ANONIMO "DESCONHECIDO"
#define TRUE 1
#define FALSE 0
#define ERRO (-1)
#define SUCESSO 0
#define SUCESSO_VOLTAR 1
#define SUCESSO_PROSSEGUIR 2
#define SUCESSO_NAO_ENCONTRADO 3
#define SIZE_STR_TEMP 256

// protocolo UDP  (CMDs msgs)
#define NUM_MAX_PARES_ARRAY 4
#define SIZE_STR_CHAVE 32
#define SIZE_STR_VALOR 512
#define PROTOCOLO_SEPARADOR_CHAVE_VALOR ":"
#define PROTOCOLO_SEPARADOR_CHAVE_VALOR_CHAR ':'
#define PROTOCOLO_SEPARADOR_PARES ";"
#define PROTOCOLO_SEPARADOR_PARES_CHAR ';'
#define CHAVE_TYPE "type"
#define CHAVE_USERNAME "username"
#define CHAVE_PASSWORD "password"
#define CHAVE_SERVICO "servico"
#define CHAVE_LOGGED "logged"
#define CHAVE_MSG "msg"
#define CHAVE_ID "id"
#define CHAVE_IP "ip"
#define CHAVE_ID_FONTE "id_fonte"
#define CHAVE_ID_DESTINO "id_destino"
#define CHAVE_IS_VALIDO "is_valido"
#define CHAVE_VALOR_CLIENTE_SERVIDOR "cliente-servidor"
#define CHAVE_VALOR_P2P "p2p"
#define CHAVE_VALOR_MULTICAST "multicast"
#define VALOR_GET_SERVICO "get_servico"
#define VALOR_LOGIN "login"
#define VALOR_SERVICO_AUTORIZADO "servico_autorizado"
#define VALOR_SERVICO_ESCOLHIDO "servico_escolhido"
#define VALOR_STATUS "status"
#define VALOR_ENVIAR_MSG "enviar_msg"
#define VALOR_ERRO "enviar_msg"
#define VALOR_TRUE "1"
#define VALOR_FALSE "0"

/* Exemplos de CMD >>  chave : valor ; ...    { opcional }
 * type: login  ;  username: ****  ;  password: ****
 * type: status ; logged: 0  ; msg: ****
 * type: erro ; msg: ****
 * type: get_servico ; username: ****
 * type: servico_autorizado  ;  p2p: 1  ;  cliente-servidor: 0  ;  multicast: 1
 * type: servico_escolhido  ;  servico: p2p  ;  id: ****        // id -> grupo ou userID
 * type: p2p  ;  ip: *****
 * type: multicast  ;  ip: *****
 * type: enviar_msg  ;  id_fonte: *****  ;  id_destino: *****  ; msg: ****
 */

// outros
#define PORTO_CLIENTE 6666
#define SIZE_STR_PACOTE 1024
#define SIZE_STR_VALOR_LITE 64
#define SIZE_STR_QUEM 32



/* MACROS */
#define h_addr h_addr_list[0]       /* for backward compatibility */



/* DEFINIÇÕES e ESTRUTURAS */
typedef int type_boolean;           // TRUE, FALSE
typedef int type_resultado;         // ERRO, SUCESSO

typedef struct {
    char chave[SIZE_STR_CHAVE];
    char valor[SIZE_STR_VALOR];
} ChaveValorRecord;



#endif //PROJAUX_HEADER_COMUM_H
