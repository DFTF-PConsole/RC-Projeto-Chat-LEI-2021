/********************************************************************************
 * "YOU MAY THINK THAT THIS FUNCTION IS OBSOLETE AND DOESNT SEEM TO DO ANYTHING *
 * AND YOU WOULD BE CORRECT BUT WHEN WE REMOVE THIS FUNTION FOR SOME REASON     *
 * THE WHOLE PROGRAM CRASHES AND WE CANT FIGURE OUT WHY SO HERE IT WILL STAY"   *
 *                                       - Fernando Pessoa (segundo a internet) *
 *  Autores:                                                                    *
 *      > Dario Felix (N.2018275530)                                            *
 *      > Vadilson Jacob (N.2015270539)                                         *
 *                                                                              *
 *  Agradecimentos:                                                             *
 *      > Google                                                                *
 *      > Stackoverflow                                                         *
 *      > IDEs                                                                  *
 *      > Take Away do Pingo Doce                                               *
 *      > Take Away do SASUC                                                    *
 *      > Cantinas do SASUC                                                     *
 *      > Cafe                                                                  *
 *      > Professores                                                           *
 *      > LUbuntu/VM VirtualBox, quando quer                                    *
 *                                                                              *
 *  Sistema de messaging                                                        *
 *  FCTUC - DEI/LEI - Redes de Comunicacao                                      *
 *  Coimbra, 25 de maio de 2021, 23:59h                                         *
 ********************************************************************************/


// FICHEIRO: servidor.c (executavel independente - servidor)



// ./servidor.out {porto clientes} {porto config} {ficheiro de registos}



/* HEADERS */
#include "header_servidor.h"



/* FUNCOES */
int main(int argc, char *argv[]) {
    signal(SIGINT, SIG_IGN);    // ignorar signal
    signal(SIGUSR1, SIG_IGN);    // ignorar signal

    snprintf(quemSouEu, SIZE_STR_QUEM, "SERVIDOR");

    intro();

    printInfo(quemSouEu, "A Inicializar...");

    if (argc != 4) {
        printErro(quemSouEu, "main", "argc != 4  >>>  ./servidor <porto clientes> <porto config> <ficheiro de registos>");
        return ERRO;
    }

    portoCliente = (short) strtol(argv[1], NULL, 10);
    portoConfig = (short) strtol(argv[2], NULL, 10);

    if (portoCliente <= 0 || portoConfig <= 0) {
        printErro(quemSouEu, "main", "erro nos portos");
        return ERRO;
    }

    snprintf(nomeFichRegisto, SIZE_STR_FICHEIRO, "%s", argv[3]);
    fpFichRegisto = NULL;

    if (isFichValido(nomeFichRegisto) == FALSE) {
        printErro(quemSouEu, "main", "erro no ficheiro");
        return ERRO;
    }

    enderecoServidor = NULL;
    enderecoCliente = NULL;
    enderecoSuplente = NULL;
    fdSocketServidor = 0;
    fdSocketCliente = 0;

    if ((pidFilho = fork()) == 0) {
        // FILHO (pid == 0)
        snprintf(quemSouEu, SIZE_STR_QUEM, SERVIDOR_FILHO);
        signal(SIGUSR1, sigusr1Handler);

        processoUDP();

        signal(SIGUSR1, SIG_IGN);
        kill(getppid(), SIGINT);
        terminar();

    } else {
        // PAI CONTINUA...
        snprintf(quemSouEu, SIZE_STR_QUEM, SERVIDOR_PAI);
        signal(SIGINT, sigintHandler);

        processoTCP();

        signal(SIGINT, SIG_IGN);
        kill(pidFilho, SIGUSR1);
        terminar();
        waitpid(pidFilho, NULL, 0);
    }

    printInfo(quemSouEu, "Terminado!");
    exit(0);
}



type_resultado processoTCP(void) {
    // LIDA COM ADMIN / CONFIG

    printInfo(quemSouEu, "A Inicializar processoTCP...");
    char printEndereco[INET_ADDRSTRLEN];
    char strTemp[SIZE_STR_TEMP];
    int enderecoSize = sizeof(struct sockaddr_in);
    type_boolean fatorConectado;
    type_boolean fatorErro;
    char pacote[SIZE_STR_PACOTE];
    ssize_t nRead;
    char listaCmd[NUM_MAX_CMDS_ARRAY][SIZE_STR_VALOR_LITE];

    if ((enderecoServidor = mallocEndereco()) == NULL)
        return ERRO;
    enderecoServidor->sin_family = AF_INET;
    enderecoServidor->sin_addr.s_addr = htonl(INADDR_ANY);
    enderecoServidor->sin_port = htons(portoConfig);

    if ((fdSocketServidor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printErro(quemSouEu, "processoTCP", "Socket < 0");
        return ERRO;
    }

    if (bind(fdSocketServidor, (struct sockaddr*) enderecoServidor, sizeof(struct sockaddr_in)) < 0) {
        printErro(quemSouEu, "processoTCP", "bind < 0");
        return ERRO;
    }

    if(listen(fdSocketServidor, 0) < 0) {
        printErro(quemSouEu, "processoTCP", "listen < 0");
        return ERRO;
    }

    inet_ntop(AF_INET, &(enderecoServidor->sin_addr.s_addr), printEndereco, INET_ADDRSTRLEN);
    snprintf(strTemp, SIZE_STR_TEMP, "processoTCP Pronto! Em escuta em %s:%d", printEndereco, ntohs(enderecoServidor->sin_port));
    printInfo(quemSouEu, strTemp);

    while (TRUE) {
        if ((enderecoCliente = mallocEndereco()) == NULL)
            return ERRO;

        fdSocketCliente = accept(fdSocketServidor, (struct sockaddr *) enderecoCliente, (socklen_t *) &enderecoSize); //wait for new connection

        if (fdSocketCliente > 0) {

            inet_ntop(AF_INET, &(enderecoCliente->sin_addr.s_addr), printEndereco, INET_ADDRSTRLEN);
            snprintf(strTemp, SIZE_STR_TEMP, "Recebido nova conexao de %s:%d", printEndereco, ntohs(enderecoCliente->sin_port));
            printInfo(quemSouEu, strTemp);

            fatorErro = FALSE;
            fatorConectado = TRUE;
            while (fatorConectado) {
                nRead = read(fdSocketCliente, pacote, SIZE_STR_PACOTE-1);
                if (nRead <= 0) {
                    printErro(quemSouEu, "processoTCP", "Ligacao Terminada Inesperadamente!");
                    fatorConectado = FALSE;
                    fatorErro = TRUE;
                } else {
                    pacote[nRead] = '\0';

                    if (getAndCheckListaCmdFromPacote(pacote, listaCmd) != ERRO) {
                        if (strcmp(listaCmd[0], CMD_QUIT) == 0) {
                            fatorConectado = FALSE;
                            snprintf(pacote, SIZE_STR_PACOTE, "Ligacao Terminada!\n");

                        } else if (strcmp(listaCmd[0], CMD_LIST) == 0) {
                            snprintf(pacote, SIZE_STR_PACOTE, "*** Lista: ***\n");
                            write(fdSocketCliente, pacote, 1 + strlen(pacote));

                            if(listFromFichRegisto() == ERRO) {
                                snprintf(pacote, SIZE_STR_PACOTE, "Erro: Nao foi possivel executar o comando!\n");
                            }

                            snprintf(pacote, SIZE_STR_PACOTE, "*** Feito! ***\n");

                        } else if (strcmp(listaCmd[0], CMD_ADD) == 0) {
                            if (addUserInFichRegisto(listaCmd[1], listaCmd[2], listaCmd[3], listaCmd[4], listaCmd[5], listaCmd[6]) == SUCESSO) {
                                snprintf(strTemp, SIZE_STR_TEMP, "UserID \"%s\" foi adicionado ao ficheiro de registos", listaCmd[1]);
                                printInfo(quemSouEu, strTemp);
                                snprintf(pacote, SIZE_STR_PACOTE, "Sucesso: ao executar o comando!\n");
                            } else {
                                snprintf(pacote, SIZE_STR_PACOTE, "Erro: Nao foi possivel executar o comando!\n");
                            }

                        } else if (strcmp(listaCmd[0], CMD_DEL) == 0) {
                            if (deleteByUserIdInFichRegisto(listaCmd[1]) == SUCESSO) {
                                snprintf(strTemp, SIZE_STR_TEMP, "UserID \"%s\" foi eliminado do ficheiro de registos", listaCmd[1]);
                                printInfo(quemSouEu, strTemp);
                                snprintf(pacote, SIZE_STR_PACOTE, "Sucesso: ao executar o comando!\n");
                            } else {
                                snprintf(pacote, SIZE_STR_PACOTE, "Erro: Nao foi possivel executar o comando!\n");
                            }

                        } else {
                            snprintf(pacote, SIZE_STR_PACOTE, "Erro: Comando nao existe!\n");
                        }

                    } else {
                        snprintf(pacote, SIZE_STR_PACOTE, "Erro: Comando invalido!\n");
                    }

                    write(fdSocketCliente, pacote, 1 + strlen(pacote));
                }
            }

            if (!fatorErro) {
                if(shutdown(fdSocketCliente, SHUT_RDWR)){
                    printErro(quemSouEu, "processoTCP", "shutdown(fdSocketCliente, SHUT_RDWR)");
                    return ERRO;
                }
            }

            close(fdSocketCliente);
            fdSocketCliente = 0;
            printInfo(quemSouEu, "Ligacao Terminada!");

        } else {
            printErro(quemSouEu, "processoTCP", "fdSocketCliente < 0");
        }

        free(enderecoCliente);
        enderecoCliente = NULL;
    }
}



type_resultado processoUDP(void) {
    // LIDA COM CLIENTES
    printInfo(quemSouEu, "A Inicializar processoUDP...");
    GrupoRecord listaGrupo[NUM_MAX_GRUPOS_ARRAY];
    char printEndereco[INET_ADDRSTRLEN];
    int enderecoSize = sizeof(struct sockaddr_in);
    char pacote[SIZE_STR_PACOTE];
    ssize_t nRead;
    char strTemp[SIZE_STR_TEMP];
    char * valorTemp = NULL;
    ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY];
    type_boolean isErro;
    char strValor[SIZE_STR_VALOR];

    inicializarListaGrupo(listaGrupo);

    if ((enderecoServidor = mallocEndereco()) == NULL)
        return ERRO;
    enderecoServidor->sin_family = AF_INET;
    enderecoServidor->sin_addr.s_addr = htonl(INADDR_ANY);
    enderecoServidor->sin_port = htons(portoCliente);

    if ((fdSocketServidor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printErro(quemSouEu, "processoUDP", "Socket < 0");
        return ERRO;
    }

    if (bind(fdSocketServidor, (struct sockaddr*) enderecoServidor, sizeof(struct sockaddr_in)) < 0) {
        printErro(quemSouEu, "processoUDP", "bind < 0");
        return ERRO;
    }

    inet_ntop(AF_INET, &(enderecoServidor->sin_addr.s_addr), printEndereco, INET_ADDRSTRLEN);
    snprintf(strTemp, SIZE_STR_TEMP, "processoUDP Pronto! Em escuta em %s:%d", printEndereco, ntohs(enderecoServidor->sin_port));
    printInfo(quemSouEu, strTemp);

    while (TRUE) {
        if ((enderecoCliente = mallocEndereco()) == NULL)
            return ERRO;

        nRead = recvfrom(fdSocketServidor, pacote, SIZE_STR_PACOTE, 0, (struct sockaddr *) enderecoCliente, (socklen_t *)&enderecoSize);

        inet_ntop(AF_INET, &(enderecoCliente->sin_addr.s_addr), printEndereco, INET_ADDRSTRLEN);
        snprintf(strTemp, SIZE_STR_TEMP, "Recebido novo pacote de %s:%d", printEndereco, ntohs(enderecoCliente->sin_port));
        printInfo(quemSouEu, strTemp);

        if (nRead <= 0) {
            printErro(quemSouEu, "processoUDP", "Erro inesperado no recvfrom(), nRead <= 0. (ignorado)");

        } else {
            pacote[nRead] = '\0';
            valorTemp = NULL;
            isErro = FALSE;

            if (getListaChaveValorFromPacote(pacote, listaChaveValor) == ERRO) {
                printErro(quemSouEu, "processoUDP", "Pacote com erros. (ignorado)");
                isErro = TRUE;

            } else {
                valorTemp = getValorByChaveFromLista(CHAVE_TYPE, listaChaveValor);

                if (valorTemp != NULL && strcmp(valorTemp, VALOR_LOGIN) == 0) {
                    if (doLogin(listaChaveValor) == ERRO) {
                        printErro(quemSouEu, "processoUDP", "doLogin() == ERRO. (ignorado)");
                        isErro = TRUE;
                    }

                } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_SERVICO_ESCOLHIDO) == 0) {
                    if (doServicoEscolhido(listaChaveValor, listaGrupo) == ERRO) {
                        printErro(quemSouEu, "processoUDP", "doServicoEscolhido() == ERRO. (ignorado)");
                        isErro = TRUE;
                    }

                } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_GET_SERVICO) == 0) {
                    if (doGetServico(listaChaveValor) == ERRO) {
                        printErro(quemSouEu, "processoUDP", "doGetServico() == ERRO. (ignorado)");
                        isErro = TRUE;
                    }

                } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_ENVIAR_MSG) == 0) {
                    if (doForwardingMsg(listaChaveValor) == ERRO) {
                        printErro(quemSouEu, "processoUDP", "doForwardingMsg() == ERRO. (ignorado)");
                        isErro = TRUE;
                    }

                } else {
                    if (valorTemp != NULL) {
                        snprintf(strTemp, SIZE_STR_TEMP, "Valor de CHAVE_TYPE desconhecido: \"%s\". (pacote ignorado)", valorTemp);
                    } else {
                        snprintf(strTemp, SIZE_STR_TEMP, "Valor de CHAVE_TYPE desconhecido: \"%s\". (pacote ignorado)", "NULL");
                    }
                    printErro(quemSouEu, "processoUDP", strTemp);
                    isErro = TRUE;
                }
            }
            if (isErro) {
                inicializarPacote(pacote);
                insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_ERRO);
                snprintf(strValor, SIZE_STR_VALOR, "Pacote com erros (foi ignorado pelo servidor). Verifique se os dados enviados estao corretos.");
                verificaAndCorrigeMsg(strValor, SIZE_STR_VALOR);
                insertChaveValorIntoPacote(pacote, CHAVE_MSG, strValor);
                if ( sendto(fdSocketServidor, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoCliente, enderecoSize) <= 0) {
                    printErro(quemSouEu, "processoUDP", "Erro inesperado no sendto(), nSend <= 0. (ignorado)");
                }
            }
        }

        if (enderecoSuplente != NULL) {
            free(enderecoSuplente);
            enderecoSuplente = NULL;
        }
        free(enderecoCliente);
        enderecoCliente = NULL;
    }
}



type_resultado doLogin(ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]) {
    int enderecoSize = sizeof(struct sockaddr_in);
    char pacote[SIZE_STR_PACOTE];
    char strValor[SIZE_STR_VALOR];
    char * username = NULL;
    char * password = NULL;
    RegistoRecord registo;
    type_resultado resultado;

    if (listaChaveValor == NULL) {
        printErro(quemSouEu, "doLogin", "params == NULL");
        return ERRO;
    }

    username = getValorByChaveFromLista(CHAVE_USERNAME, listaChaveValor);
    password = getValorByChaveFromLista(CHAVE_PASSWORD, listaChaveValor);

    inicializarPacote(pacote);

    if (username == NULL || password == NULL || (resultado = getRegistoByUserIdFromFichRegisto(username, &registo)) == ERRO || resultado == SUCESSO_NAO_ENCONTRADO ) {
        if (username != NULL && password != NULL && resultado == SUCESSO_NAO_ENCONTRADO) {
            snprintf(strValor, SIZE_STR_VALOR, "Username nao encontrado.");
            verificaAndCorrigeMsg(strValor, SIZE_STR_VALOR);
            insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_STATUS);
            insertChaveValorIntoPacote(pacote, CHAVE_LOGGED, VALOR_FALSE);
            insertChaveValorIntoPacote(pacote, CHAVE_MSG, strValor);

        } else {
            return ERRO;
        }

    } else {
        if (strcmp(username, registo.userID) == 0 && strcmp(password, registo.password) == 0) {
            snprintf(strValor, SIZE_STR_VALOR, "Login efetuado com sucesso.");
            verificaAndCorrigeMsg(strValor, SIZE_STR_VALOR);
            insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_STATUS);
            insertChaveValorIntoPacote(pacote, CHAVE_LOGGED, VALOR_TRUE);
            insertChaveValorIntoPacote(pacote, CHAVE_MSG, strValor);

        } else {
            snprintf(strValor, SIZE_STR_VALOR, "Username e password nao coincidem.");
            verificaAndCorrigeMsg(strValor, SIZE_STR_VALOR);
            insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_STATUS);
            insertChaveValorIntoPacote(pacote, CHAVE_LOGGED, VALOR_FALSE);
            insertChaveValorIntoPacote(pacote, CHAVE_MSG, strValor);
        }
    }

    if ( sendto(fdSocketServidor, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoCliente, enderecoSize) <= 0) {
        printErro(quemSouEu, "doLogin", "Erro inesperado no sendto(), nSend <= 0");
        return ERRO;
    }

    return SUCESSO;
}



type_resultado doGetServico(ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]) {
    int enderecoSize = sizeof(struct sockaddr_in);
    char pacote[SIZE_STR_PACOTE];
    char * username  = NULL;
    RegistoRecord registo;
    char strValor[SIZE_STR_VALOR];
    type_resultado resultado;

    if (listaChaveValor == NULL) {
        printErro(quemSouEu, "doGetServico", "params == NULL");
        return ERRO;
    }

    username = getValorByChaveFromLista(CHAVE_USERNAME, listaChaveValor);    // pode ser NULL

    inicializarPacote(pacote);

    if (username == NULL || (resultado = getRegistoByUserIdFromFichRegisto(username, &registo)) == ERRO || resultado == SUCESSO_NAO_ENCONTRADO) {
        return ERRO;
    }

    insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_SERVICO_AUTORIZADO);

    snprintf(strValor, SIZE_STR_VALOR, "%d", registo.clienteServidor);
    verificaAndCorrigeMsg(strValor, SIZE_STR_VALOR);
    insertChaveValorIntoPacote(pacote, CHAVE_VALOR_CLIENTE_SERVIDOR, strValor);

    snprintf(strValor, SIZE_STR_VALOR, "%d", registo.p2p);
    verificaAndCorrigeMsg(strValor, SIZE_STR_VALOR);
    insertChaveValorIntoPacote(pacote, CHAVE_VALOR_P2P, strValor);

    snprintf(strValor, SIZE_STR_VALOR, "%d", registo.multicast);
    verificaAndCorrigeMsg(strValor, SIZE_STR_VALOR);
    insertChaveValorIntoPacote(pacote, CHAVE_VALOR_MULTICAST, strValor);

    if ( sendto(fdSocketServidor, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoCliente, enderecoSize) <= 0) {
        printErro(quemSouEu, "doGetServico", "Erro inesperado no sendto(), nSend <= 0");
        return ERRO;
    }

    return SUCESSO;
}



type_resultado doServicoEscolhido(ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY], GrupoRecord listaGrupo[NUM_MAX_GRUPOS_ARRAY]) {
    int enderecoSize = sizeof(struct sockaddr_in);
    char pacote[SIZE_STR_PACOTE];
    char * servico = NULL;
    char * idUserOuGrupo  = NULL;
    char ipMulticast[INET_ADDRSTRLEN];
    RegistoRecord registo;
    type_resultado resultado;

    if (listaChaveValor == NULL || listaGrupo == NULL) {
        printErro(quemSouEu, "doServicoEscolhido", "params == NULL");
        return ERRO;
    }

    servico = getValorByChaveFromLista(CHAVE_SERVICO, listaChaveValor);
    idUserOuGrupo = getValorByChaveFromLista(CHAVE_ID, listaChaveValor);    // pode ser NULL

    inicializarPacote(pacote);

    if (servico != NULL && idUserOuGrupo != NULL && strcmp(servico, CHAVE_VALOR_P2P) == 0) {
        if ((resultado = getRegistoByUserIdFromFichRegisto(idUserOuGrupo, &registo)) == ERRO || resultado == SUCESSO_NAO_ENCONTRADO) {
            return ERRO;
        }
        insertChaveValorIntoPacote(pacote, CHAVE_TYPE, CHAVE_VALOR_P2P);
        insertChaveValorIntoPacote(pacote, CHAVE_IP, registo.ip);

    } else if (servico != NULL && idUserOuGrupo != NULL && strcmp(servico, CHAVE_VALOR_MULTICAST) == 0) {
        if (getAndSetIpMulticastByGrupoIdFromLista(listaGrupo, idUserOuGrupo, ipMulticast) == ERRO) {
            return ERRO;
        }
        insertChaveValorIntoPacote(pacote, CHAVE_TYPE, CHAVE_VALOR_MULTICAST);
        insertChaveValorIntoPacote(pacote, CHAVE_IP, ipMulticast);

    } else if (servico != NULL && idUserOuGrupo != NULL && strcmp(servico, CHAVE_VALOR_CLIENTE_SERVIDOR) == 0) {
        if ((resultado = getRegistoByUserIdFromFichRegisto(idUserOuGrupo, &registo)) == ERRO) {
            return ERRO;
        }
        insertChaveValorIntoPacote(pacote, CHAVE_TYPE, CHAVE_VALOR_CLIENTE_SERVIDOR);
        if (resultado == SUCESSO) {
            insertChaveValorIntoPacote(pacote, CHAVE_IS_VALIDO, VALOR_TRUE);
        } else {
            insertChaveValorIntoPacote(pacote, CHAVE_IS_VALIDO, VALOR_FALSE);
        }

    } else {
        printErro(quemSouEu, "doServicoEscolhido", "servico desconhecido");
        return ERRO;
    }

    if ( sendto(fdSocketServidor, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoCliente, enderecoSize) <= 0) {
        printErro(quemSouEu, "doServicoEscolhido", "Erro inesperado no sendto(), nSend <= 0");
        return ERRO;
    }

    return SUCESSO;
}



type_resultado doForwardingMsg(ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY]) {
    int enderecoSize = sizeof(struct sockaddr_in);
    char pacote[SIZE_STR_PACOTE];
    char * usernameDestino = NULL;
    struct hostent * ptrIp = NULL;
    RegistoRecord registo;
    type_resultado resultado;

    if (listaChaveValor == NULL) {
        printErro(quemSouEu, "doForwardingMsg", "params == NULL");
        return ERRO;
    }

    usernameDestino = getValorByChaveFromLista(CHAVE_ID_DESTINO, listaChaveValor);

    if (enderecoSuplente != NULL) {
        free(enderecoSuplente);
        enderecoSuplente = NULL;
    }
    if ((enderecoSuplente = mallocEndereco()) == NULL) {
        printErro(quemSouEu, "doForwardingMsg", "mallocEndereco() == NULL");
        return ERRO;
    }

    if (usernameDestino == NULL || (resultado = getRegistoByUserIdFromFichRegisto(usernameDestino, &registo)) == ERRO || resultado == SUCESSO_NAO_ENCONTRADO ) {
        free(enderecoSuplente);
        enderecoSuplente = NULL;
        return ERRO;
    }

    if ((ptrIp = gethostbyname(registo.ip)) == 0) {
        printErro(quemSouEu, "doForwardingMsg", "gethostbyname() == NULL, impossivel obter endereco");
        free(enderecoSuplente);
        enderecoSuplente = NULL;
        return ERRO;
    }

    enderecoSuplente->sin_family = AF_INET;
    enderecoSuplente->sin_addr.s_addr = ((struct in_addr *)(ptrIp->h_addr))->s_addr;
    enderecoSuplente->sin_port = htons(PORTO_CLIENTE);

    if (getPacoteFromListaChaveValor (pacote, listaChaveValor) == ERRO) {
        printErro(quemSouEu, "doForwardingMsg", "ao criar pacote, getPacoteFromListaChaveValor() == ERRO");
        free(enderecoSuplente);
        enderecoSuplente = NULL;
        return ERRO;
    }

    if ( sendto(fdSocketServidor, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoSuplente, enderecoSize) <= 0) {
        printErro(quemSouEu, "doForwardingMsg", "Erro inesperado no sendto(), nSend <= 0");
        free(enderecoSuplente);
        enderecoSuplente = NULL;
        return ERRO;
    }

    free(enderecoSuplente);
    enderecoSuplente = NULL;

    return SUCESSO;
}



type_resultado getAndSetIpMulticastByGrupoIdFromLista(GrupoRecord listaGrupo[NUM_MAX_GRUPOS_ARRAY], const char grupoID[SIZE_STR_VALOR_LITE], char ipMulticast[INET_ADDRSTRLEN]) {
    int i;
    type_boolean fatorFor;

    if (listaGrupo == NULL || grupoID == NULL || ipMulticast == NULL) {
        printErro(quemSouEu, "getAndSetIpMulticastByGrupoIdFromLista", "params == NULL");
        return ERRO;
    }

    for (i=1, fatorFor=TRUE; fatorFor && i < NUM_MAX_GRUPOS_ARRAY ; i++) {
        if (strcmp(listaGrupo[i].grupoID, grupoID) == 0) {
            snprintf(ipMulticast, INET_ADDRSTRLEN, "%s", listaGrupo[i].ipMulticast);
            fatorFor = FALSE;
        } else {
            if (listaGrupo[i].grupoID[0] =='\0') {
                snprintf(listaGrupo[i].grupoID, SIZE_STR_VALOR_LITE, "%s", grupoID);
                snprintf(ipMulticast, INET_ADDRSTRLEN, "%s", listaGrupo[i].ipMulticast);
                fatorFor = FALSE;
            }
        }
    }

    if (i >= NUM_MAX_GRUPOS_ARRAY) {
        snprintf(ipMulticast, INET_ADDRSTRLEN, "%s", listaGrupo[0].ipMulticast);
        return ERRO;
    }

    return SUCESSO;
}



void inicializarListaGrupo(GrupoRecord listaGrupo[NUM_MAX_GRUPOS_ARRAY]) {
    int i;

    if (listaGrupo == NULL) {
        printErro(quemSouEu, "inicializarListaGrupo", "params == NULL");
        return;
    }

    for (i=0 ; i < NUM_MAX_GRUPOS_ARRAY ; i++) {
        listaGrupo[i].grupoID[0] = '\0';
        snprintf(listaGrupo[i].ipMulticast, INET_ADDRSTRLEN, "%s%d", IP_MULTICAST_BASE, i+1);
    }

    snprintf(listaGrupo[0].grupoID, SIZE_STR_VALOR_LITE, "%s", GRUPO_ID_DEFAULT);
}



void sigintHandler(int signum) {
    // PAI

    if (signum == SIGINT) {
        putchar('\n');
        printInfo(quemSouEu, "SIGINT Recebido!");
        signal(SIGINT, SIG_IGN);    // ignorar signal

        terminar();

        printInfo(quemSouEu, "Processo Principal Terminado! Em espera que os processos-filhos terminem...");
        kill(pidFilho, SIGUSR1);
        waitpid(pidFilho,NULL,0);

        printInfo(quemSouEu, "Todos os Processos Foram Terminados!");
        exit(0);
    }
}



void sigusr1Handler(int signum) {
    // FILHO

    if (signum == SIGUSR1) {
        printInfo(quemSouEu, "SIGUSR1 Recebido!");
        signal(SIGUSR1, SIG_IGN);    // ignorar signal

        terminar();

        printInfo(quemSouEu, "Processo Terminado!");
        exit(0);
    }
}



void terminar(void) {
    if (fpFichRegisto != NULL)
        fclose(fpFichRegisto);
    if (enderecoServidor != NULL)
        free(enderecoServidor);
    if (enderecoCliente != NULL)
        free(enderecoCliente);
    if (enderecoSuplente != NULL)
        free(enderecoSuplente);
    if (fdSocketServidor > 0)
        close(fdSocketServidor);
    if (fdSocketCliente > 0)
        close(fdSocketCliente);

    printInfo(quemSouEu, "File descriptor (Sockets) fechados; file pointer (FILE) fechado; e memoria alocada (Malloc) libertada");
}



type_boolean isFichValido(const char * nomefichTemp) {
    // verifica se existe
    if (nomefichTemp == NULL) {
        printErro(quemSouEu, "isFichValido", "params == NULL");
        return FALSE;
    }

    struct stat bufferTemp;
    int existeTemp = stat(nomefichTemp, &bufferTemp);

    FILE * fichTemp = NULL;

    if(existeTemp != 0) {
        // 1. FALSE

        // verifica se pode criar
        if ((fichTemp = fopen(nomefichTemp, "wb"))){
            // 2. TRUE
            fclose(fichTemp);

        } else {    // == 0
            return FALSE;   // 2. FALSE
        }
    }
    // 1. TRUE  (e 2.)

    return TRUE;
}



type_resultado getListaCmdFromPacote (const char pacote[SIZE_STR_PACOTE], char listaCmd[NUM_MAX_CMDS_ARRAY][SIZE_STR_VALOR_LITE]) {
    int i;
    char * token = NULL;
    char pacoteTemp [SIZE_STR_PACOTE];
    size_t count;

    if (listaCmd == NULL || pacote == NULL) {
        printErro(quemSouEu, "getListaCmdFromPacote", "params == NULL");
        return ERRO;
    }

    snprintf(pacoteTemp, SIZE_STR_PACOTE, "%s", pacote);
    count = strlen(pacoteTemp);
    if (pacoteTemp[count - 1] == '\n' || pacoteTemp[count - 1] == '\r') {
        pacoteTemp[count - 1] = '\0';
    } else {
        pacoteTemp[count - 1] = pacoteTemp[count - 1];
    }
    if (pacoteTemp[count - 2] == '\n' || pacoteTemp[count - 1] == '\r') {
        pacoteTemp[count - 2] = '\0';
    } else {
        pacoteTemp[count - 2] = pacoteTemp[count - 2];
    }

    token = strtok(pacoteTemp, PROTOCOLO_SEPARADOR_CMDS);
    for(i=0 ; token != NULL && i < NUM_MAX_CMDS_ARRAY ; i++) {
        if (snprintf(listaCmd[i], SIZE_STR_VALOR_LITE, "%s", token) < 0){
            printErro(quemSouEu, "getListaCmdFromPacote", "listaCmd: snprintf() < 0");
            return ERRO;
        }
        token = strtok(NULL, PROTOCOLO_SEPARADOR_CMDS);
    }

    if (token != NULL || i == 0) {
        printErro(quemSouEu, "getListaCmdFromPacote", "token != NULL || i == 0");
        return ERRO;
    }

    for( ; i < NUM_MAX_CMDS_ARRAY ; i++) {
        listaCmd[i][0] = '\0';
    }

    return SUCESSO;
}



type_resultado getAndCheckListaCmdFromPacote (const char pacote[SIZE_STR_PACOTE], char listaCmd[NUM_MAX_CMDS_ARRAY][SIZE_STR_VALOR_LITE]) {
    if (pacote == NULL || listaCmd == NULL) {
        printErro(quemSouEu, "getAndCheckListaCmdFromPacote", "params == NULL");
        return ERRO;
    }

    if (getListaCmdFromPacote(pacote, listaCmd) != ERRO) {
        if (checkListaCmd(listaCmd) != ERRO) {
            return SUCESSO;
        }
    }

    return ERRO;
}



type_resultado checkListaCmd (const char listaCmd[NUM_MAX_CMDS_ARRAY][SIZE_STR_VALOR_LITE]) {
    int i, max = 7;
    type_boolean fator = ERRO;

    if (listaCmd == NULL) {
        printErro(quemSouEu, "checkListaCmd", "params == NULL");
        return ERRO;
    }

    if ((strcmp(listaCmd[0], CMD_LIST) == 0 && listaCmd[1][0] == '\0') || (strcmp(listaCmd[0], CMD_QUIT) == 0 && listaCmd[1][0] == '\0') || (strcmp(listaCmd[0], CMD_DEL) == 0 && listaCmd[1][0] != '\0' && listaCmd[2][0] == '\0')) {
        fator = SUCESSO;

    } else if (strcmp(listaCmd[0], CMD_ADD) == 0) {
        for (i=1 ; i < 4 && i < NUM_MAX_CMDS_ARRAY ; i++) {
            if (listaCmd[i][0] == '\0') {
                printErro(quemSouEu, "checkListaCmd", "Erro no Cmds: CMD_ADD");
                return ERRO;
            }
        }

        for (; i < max && i < NUM_MAX_CMDS_ARRAY ; i++) {
            if (strcmp(listaCmd[i], CMD_YES) != 0 && strcmp(listaCmd[i], CMD_NO) != 0) {
                printErro(quemSouEu, "checkListaCmd", "Erro no Cmds: CMD_ADD");
                return ERRO;
            }
        }

        fator = SUCESSO;
    }

    if (fator == SUCESSO) {
        for (i=0 ; i < NUM_MAX_CMDS_ARRAY ; i++) {
            if (verificaMsg(listaCmd[i], SIZE_STR_VALOR_LITE) == TRUE) {
                printErro(quemSouEu, "checkListaCmd", "Cmds com chars invalidos");
                return ERRO;
            }
        }
        return SUCESSO;
    }

    printErro(quemSouEu, "checkListaCmd", "Erro no Cmds");
    return ERRO;
}



type_resultado deleteByUserIdInFichRegisto (const char userID[SIZE_STR_VALOR_LITE]) {
    FILE * fpTempFile = NULL;
    char pacote[SIZE_STR_PACOTE];
    RegistoRecord registo;
    type_boolean isDelete;
    type_boolean fatorWhile;
    size_t nRead;

    if (userID == NULL) {
        printErro(quemSouEu, "deleteByUserIdInFichRegisto", "params == NULL");
        return ERRO;
    }

    if ((fpFichRegisto = fopen(nomeFichRegisto, "rb")) == NULL){
        printErro(quemSouEu, "deleteByUserIdInFichRegisto", "fpFichRegisto == NULL");
        return ERRO;
    }

    if ((fpTempFile = fopen(NOME_FICH_TEMP, "wb")) == NULL){
        printErro(quemSouEu, "deleteByUserIdInFichRegisto", "fpTempFile == NULL");
        fclose(fpFichRegisto);
        fpFichRegisto = NULL;
        return ERRO;
    }

    fseek(fpFichRegisto, 0, SEEK_SET);

    isDelete = fatorWhile = TRUE;
    while ( fatorWhile ) {

        nRead = fread(&registo, sizeof(RegistoRecord), 1, fpFichRegisto);

        if (nRead != 1) {
            if (feof(fpFichRegisto)) {  // EOF
                fatorWhile = FALSE;
            } else if (ferror(fpFichRegisto)) { // ERRO
                printErro(quemSouEu, "deleteByUserIdInFichRegisto", "ferror(fpFichRegisto)");
                fclose(fpFichRegisto);
                fpFichRegisto = NULL;
                fclose(fpTempFile);
                return ERRO;
            } else {
                printErro(quemSouEu, "deleteByUserIdInFichRegisto", "erro desconhecido nos ficheiros");
                fclose(fpFichRegisto);
                fpFichRegisto = NULL;
                fclose(fpTempFile);
                return ERRO;
            }

        } else {    // SUCESSO
            if (strcmp(registo.userID, userID) == 0) {
                isDelete = FALSE;
            } else {
                nRead = fwrite(&registo, sizeof(RegistoRecord), 1, fpTempFile);
                if (nRead != 1) {
                    printErro(quemSouEu, "deleteByUserIdInFichRegisto", "erro desconhecido no fwrite(fpTempFile)");
                    fclose(fpFichRegisto);
                    fpFichRegisto = NULL;
                    fclose(fpTempFile);
                    return ERRO;
                }
            }
        }
    }

    fclose(fpFichRegisto);
    fpFichRegisto = NULL;
    fclose(fpTempFile);

    if (remove(nomeFichRegisto)) {
        printErro(quemSouEu, "deleteByUserIdInFichRegisto", "remove(nomeFichRegisto)");
        return ERRO;
    }

    if (rename(NOME_FICH_TEMP, nomeFichRegisto)) {
        printErro(quemSouEu, "deleteByUserIdInFichRegisto", "rename(NOME_FICH_TEMP, nomeFichRegisto)");
        return ERRO;
    }

    if (isDelete) {
        snprintf(pacote, SIZE_STR_PACOTE, "Nenhum user encontrado com esse ID (\"%s\")!\n", userID);
    } else {
        snprintf(pacote, SIZE_STR_PACOTE, "User \"%s\" eliminado!\n", userID);
    }

    write(fdSocketCliente, pacote, 1 + strlen(pacote));

    return SUCESSO;
}



type_resultado addUserInFichRegisto (const char userID[SIZE_STR_VALOR_LITE], const char ip[SIZE_STR_VALOR_LITE], const char password[SIZE_STR_VALOR_LITE], const char clienteServidor[SIZE_STR_VALOR_LITE], const char p2p[SIZE_STR_VALOR_LITE], const char multicast[SIZE_STR_VALOR_LITE]) {
    struct sockaddr_in endereco;
    char pacote[SIZE_STR_PACOTE];
    RegistoRecord registo;
    RegistoRecord registoTemp;
    size_t nRead;
    type_resultado resultado;

    if (userID == NULL || ip == NULL || password == NULL || clienteServidor == NULL || p2p == NULL || multicast == NULL) {
        printErro(quemSouEu, "addUserInFichRegisto", "params == NULL");
        return ERRO;
    }

    snprintf(registo.userID, SIZE_STR_VALOR_LITE, "%s", userID);
    snprintf(registo.ip, INET_ADDRSTRLEN, "%s", ip);
    snprintf(registo.password, SIZE_STR_VALOR_LITE, "%s", password);
    registo.clienteServidor = (strcmp(clienteServidor, CMD_YES) == 0) ? TRUE : FALSE;
    registo.p2p = (strcmp(p2p, CMD_YES) == 0) ? TRUE : FALSE;
    registo.multicast = (strcmp(multicast, CMD_YES) == 0) ? TRUE : FALSE;

    if (inet_pton(AF_INET, registo.ip, &(endereco.sin_addr)) != 1) {
        snprintf(pacote, SIZE_STR_PACOTE, "Erro no IP (\"%s\")\n", registo.ip);
        write(fdSocketCliente, pacote, 1 + strlen(pacote));
        printErro(quemSouEu, "addUserInFichRegisto", "Erro no IP");
        return ERRO;
    }

    if ((resultado = getRegistoByUserIdFromFichRegisto(userID, &registoTemp)) == ERRO ) {
        return ERRO; // erro grave

    } else {
        if (resultado == SUCESSO) {
            snprintf(pacote, SIZE_STR_PACOTE, "userID ja se encontra registado (\"%s\")\n", userID);
            write(fdSocketCliente, pacote, 1 + strlen(pacote));
            printErro(quemSouEu, "addUserInFichRegisto", "userID ja registado");
            return ERRO;
        }
    }

    if ((fpFichRegisto = fopen(nomeFichRegisto, "rb+")) == NULL){
        printErro(quemSouEu, "addUserInFichRegisto", "fpFichRegisto == NULL");
        return ERRO;
    }

    fseek(fpFichRegisto, 0, SEEK_END);

    nRead = fwrite(&registo, sizeof(RegistoRecord), 1, fpFichRegisto);
    if (nRead != 1) {
        printErro(quemSouEu, "addUserInFichRegisto", "erro desconhecido no fwrite(fpFichRegisto)");
        fclose(fpFichRegisto);
        fpFichRegisto = NULL;
        return ERRO;
    }

    fclose(fpFichRegisto);
    fpFichRegisto = NULL;

    snprintf(pacote, SIZE_STR_PACOTE, "User \"%s\" Adicionado!\n", userID);
    write(fdSocketCliente, pacote, 1 + strlen(pacote));

    return SUCESSO;
}



type_resultado listFromFichRegisto (void) {
    char pacote[SIZE_STR_PACOTE];
    RegistoRecord registo;
    char yesOrNo[3][SIZE_STR_VALOR_LITE];
    type_boolean isVazio;
    type_boolean fatorWhile;
    size_t nRead;

    if ((fpFichRegisto = fopen(nomeFichRegisto, "rb")) == NULL){
        printErro(quemSouEu, "listFromFichRegisto", "fpFichRegisto == NULL");
        return ERRO;
    }

    snprintf(pacote, SIZE_STR_PACOTE, " USER_ID   IP   PASSWORD   CLIENTE-SERVIDOR   P2P   GRUPO\n");
    write(fdSocketCliente, pacote, 1 + strlen(pacote));

    fseek(fpFichRegisto, 0, SEEK_SET);

    isVazio = fatorWhile = TRUE;
    while ( fatorWhile ) {

        nRead = fread(&registo, sizeof(RegistoRecord), 1, fpFichRegisto);

        if (nRead != 1) {
            if (feof(fpFichRegisto)) {  // EOF
                fatorWhile = FALSE;
            } else if (ferror(fpFichRegisto)) { // ERRO
                printErro(quemSouEu, "listFromFichRegisto", "ferror(fpFichRegisto)");
                fclose(fpFichRegisto);
                fpFichRegisto = NULL;
                return ERRO;
            } else {
                printErro(quemSouEu, "listFromFichRegisto", "erro desconhecido nos ficheiros");
                fclose(fpFichRegisto);
                fpFichRegisto = NULL;
                return ERRO;
            }

        } else {    // SUCESSO
            isVazio = FALSE;

            if (registo.clienteServidor == TRUE) {
                snprintf(yesOrNo[0], SIZE_STR_VALOR_LITE, "%s", CMD_YES);
            } else {
                snprintf(yesOrNo[0], SIZE_STR_VALOR_LITE, "%s", CMD_NO);
            }

            if (registo.p2p == TRUE) {
                snprintf(yesOrNo[1], SIZE_STR_VALOR_LITE, "%s", CMD_YES);
            } else {
                snprintf(yesOrNo[1], SIZE_STR_VALOR_LITE, "%s", CMD_NO);
            }

            if (registo.multicast == TRUE) {
                snprintf(yesOrNo[2], SIZE_STR_VALOR_LITE, "%s", CMD_YES);
            } else {
                snprintf(yesOrNo[2], SIZE_STR_VALOR_LITE, "%s", CMD_NO);
            }

            snprintf(pacote, SIZE_STR_PACOTE, " %s   %s   %s   %s   %s   %s\n", registo.userID, registo.ip, registo.password, yesOrNo[0], yesOrNo[1], yesOrNo[2]);
            write(fdSocketCliente, pacote, 1 + strlen(pacote));
        }
    }

    if (isVazio) {
        snprintf(pacote, SIZE_STR_PACOTE, " Sem clientes registados!\n");
        write(fdSocketCliente, pacote, 1 + strlen(pacote));
    }

    fclose(fpFichRegisto);
    fpFichRegisto = NULL;

    return SUCESSO;
}



type_resultado getRegistoByUserIdFromFichRegisto (const char userID[SIZE_STR_VALOR_LITE], RegistoRecord * registo) {
    type_boolean fatorWhile, fatorEncontrado;
    size_t nRead;

    if (registo == NULL || userID == NULL) {
        printErro(quemSouEu, "getRegistoByUserIdFromFichRegisto", "params == NULL");
        return ERRO;
    }

    if ((fpFichRegisto = fopen(nomeFichRegisto, "rb")) == NULL){
        printErro(quemSouEu, "getRegistoByUserIdFromFichRegisto", "fpFichRegisto == NULL");
        return ERRO;
    }

    fseek(fpFichRegisto, 0, SEEK_SET);

    fatorWhile = TRUE;
    fatorEncontrado = FALSE;
    while ( fatorWhile ) {
        nRead = fread(registo, sizeof(RegistoRecord), 1, fpFichRegisto);

        if (nRead != 1) {
            if (feof(fpFichRegisto)) {  // EOF
                fatorWhile = FALSE;
            } else if (ferror(fpFichRegisto)) { // ERRO
                printErro(quemSouEu, "getRegistoByUserIdFromFichRegisto", "ferror(fpFichRegisto)");
                fclose(fpFichRegisto);
                fpFichRegisto = NULL;
                return ERRO;
            } else {
                printErro(quemSouEu, "getRegistoByUserIdFromFichRegisto", "erro desconhecido nos ficheiros");
                fclose(fpFichRegisto);
                fpFichRegisto = NULL;
                return ERRO;
            }

        } else {    // SUCESSO
            if (strcmp(registo->userID, userID) == 0) {
                fatorWhile = FALSE;
                fatorEncontrado = TRUE;
            }
        }
    }

    fclose(fpFichRegisto);
    fpFichRegisto = NULL;

    if (!fatorEncontrado) {
        printAviso(quemSouEu, "getRegistoByUserIdFromFichRegisto", "UserID nao encontrado");
        return SUCESSO_NAO_ENCONTRADO;
    }

    return SUCESSO;
}


