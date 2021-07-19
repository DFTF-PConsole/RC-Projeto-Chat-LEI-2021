/*  AUTORES: Dário Félix N.2018275530 | Vadilson Jacob N.2015270539  */

// FICHEIRO: header_cliente.h (para cliente.c)



#ifndef PROJAUX_HEADER_CLIENTE_H
#define PROJAUX_HEADER_CLIENTE_H



/* BIBLIOTECAS */
#include "header_comum.h"
#include "header_global.h"




/* CONSTANTES */
#define SERVIDOR "SERVIDOR"
#define CLIENTE "CLIENTE"
#define CLIENTE_RECETOR "CLIENTE-RECETOR"       // Processo 2 / Filho
#define CLIENTE_EMISSOR "CLIENTE-EMISSOR"       // Processo 1 / Principal / Pai



/* MACROS */



/* DEFINIÇÕES e ESTRUTURAS */
typedef struct {
    type_boolean clienteServidor;
    type_boolean p2p;
    type_boolean multicast;
} Servico;



/* VARIAVEIS GLOBAIS */
pid_t pidFilho;
char quemSouEu[SIZE_STR_QUEM];
short portoServidor;
struct sockaddr_in * enderecoEscuta;   // fazer bind
struct sockaddr_in * enderecoReceber;
struct sockaddr_in * enderecoEnviar;
int fdSocketEscuta;
char ipServidor[INET_ADDRSTRLEN];
char username[SIZE_STR_VALOR_LITE];
char usernameDestino[SIZE_STR_VALOR_LITE];
char ipDestino[INET_ADDRSTRLEN];



/* PROTOTIPOS */
void sigintHandlerMain(int signum);
void sigintHandler(int signum);
void sigusr1Handler(int signum);
void terminar(void);
type_resultado processoRecetorUDP(void);
type_resultado processoEmissorUDP(void);
type_resultado doLogin(void);
type_resultado doServico(void);
type_resultado conectaServidor(void);
type_resultado conectaP2p (void);
type_resultado conectaMulticast (void);
type_resultado conectaLimpar (void);
void printMsgChat (const char * idFonte, const char * idDestino, const char * ip, int porto, const char * msg);



#endif //PROJAUX_HEADER_CLIENTE_H
