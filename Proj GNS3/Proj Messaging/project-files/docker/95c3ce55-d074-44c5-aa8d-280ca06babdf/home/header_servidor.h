/*  AUTORES: Dário Félix N.2018275530 | Vadilson Jacob N.2015270539  */

// FICHEIRO: header_servidor.h (para servidor.c)



#ifndef PROJAUX_HEADER_SERVIDOR_H
#define PROJAUX_HEADER_SERVIDOR_H



/* BIBLIOTECAS */
#include "header_comum.h"
#include "header_global.h"



/* CONSTANTES */
// ficheiros
#define SIZE_STR_FICHEIRO 128
#define NOME_FICH_TEMP "ficheiroRegistoTemporario.byte"

// protocolo TCP (CMDs config)
#define CMD_LIST "LIST"
#define CMD_ADD "ADD"
#define CMD_DEL "DEL"
#define CMD_QUIT "QUIT"
#define NUM_MAX_CMDS_ARRAY 7
#define PROTOCOLO_SEPARADOR_CMDS " "
#define CMD_YES "yes"
#define CMD_NO "no"

// outros
#define IP_MULTICAST_BASE "235.1.1."
#define SERVIDOR_PAI "SERVIDOR-CONFIG"
#define SERVIDOR_FILHO "SERVIDOR-CLIENTE"
#define NUM_MAX_GRUPOS_ARRAY 10
#define GRUPO_ID_DEFAULT "default"



/* MACROS */



/* DEFINIÇÕES e ESTRUTURAS */
typedef struct {
    char userID[SIZE_STR_VALOR_LITE];
    char password[SIZE_STR_VALOR_LITE];
    char ip[INET_ADDRSTRLEN];
    type_boolean clienteServidor;
    type_boolean p2p;
    type_boolean multicast;
} RegistoRecord;

typedef struct {
    char grupoID[SIZE_STR_VALOR_LITE];
    char ipMulticast[INET_ADDRSTRLEN];
} GrupoRecord;



/* VARIAVEIS GLOBAIS */
pid_t pidFilho;
char quemSouEu[SIZE_STR_QUEM];
short portoConfig;
short portoCliente;
char nomeFichRegisto[SIZE_STR_FICHEIRO];
FILE * fpFichRegisto;
struct sockaddr_in * enderecoServidor;
struct sockaddr_in * enderecoCliente;
struct sockaddr_in * enderecoSuplente;
int fdSocketServidor;
int fdSocketCliente;



/* PROTOTIPOS */
type_resultado processoTCP(void);
type_resultado processoUDP(void);
type_resultado getAndSetIpMulticastByGrupoIdFromLista(GrupoRecord listaGrupo[NUM_MAX_GRUPOS_ARRAY], const char grupoID[SIZE_STR_VALOR_LITE], char ipMulticast[INET_ADDRSTRLEN]);
void inicializarListaGrupo(GrupoRecord listaGrupo[NUM_MAX_GRUPOS_ARRAY]);
void sigintHandler(int signum);
void sigusr1Handler(int signum);
void terminar(void);
type_boolean isFichValido(const char * nomefichTemp);
type_resultado getListaCmdFromPacote(const char pacote[SIZE_STR_PACOTE], char listaCmd[NUM_MAX_CMDS_ARRAY][SIZE_STR_VALOR_LITE]);
type_resultado checkListaCmd(const char listaCmd[NUM_MAX_CMDS_ARRAY][SIZE_STR_VALOR_LITE]);
type_resultado getAndCheckListaCmdFromPacote(const char pacote[SIZE_STR_PACOTE], char listaCmd[NUM_MAX_CMDS_ARRAY][SIZE_STR_VALOR_LITE]);
type_resultado deleteByUserIdInFichRegisto(const char userID[SIZE_STR_VALOR_LITE]);
type_resultado addUserInFichRegisto(const char userID[SIZE_STR_VALOR_LITE], const char ip[SIZE_STR_VALOR_LITE], const char password[SIZE_STR_VALOR_LITE], const char clienteServidor[SIZE_STR_VALOR_LITE], const char p2p[SIZE_STR_VALOR_LITE], const char multicast[SIZE_STR_VALOR_LITE]);
type_resultado listFromFichRegisto(void);
type_resultado getRegistoByUserIdFromFichRegisto (const char userID[SIZE_STR_VALOR_LITE], RegistoRecord * registo);
type_resultado doLogin(ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]);
type_resultado doServicoEscolhido(ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY], GrupoRecord listaGrupo[NUM_MAX_GRUPOS_ARRAY]);
type_resultado doForwardingMsg(ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]);
type_resultado doGetServico(ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]);



#endif //PROJAUX_HEADER_SERVIDOR_H
