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


// FICHEIRO: cliente.c (executavel independente - cliente)



// ./cliente.out {endereço do servidor} {porto do servidor}



/* HEADERS */
#include "header_cliente.h"



/* FUNCOES */
int main(int argc, char *argv[]) {
    type_boolean fatorWhileLogin;
    type_boolean fatorWhileServico;
    type_boolean fatorIniciaServico;
    type_resultado resultado;
    char strTemp[SIZE_STR_TEMP];
    char printEnderecoEscuta[INET_ADDRSTRLEN];
    char printEnderecoEnviar[INET_ADDRSTRLEN];

    signal(SIGINT, SIG_IGN);    // ignorar signal
    signal(SIGUSR1, SIG_IGN);    // ignorar signal

    snprintf(quemSouEu, SIZE_STR_QUEM, CLIENTE);

    intro();

    printInfo(quemSouEu, "A Inicializar...");

    if (argc != 3) {
        printErro(quemSouEu, "main", "argc != 3  >>>  ./cliente  {endereco do servidor}  {porto do servidor}");
        return ERRO;
    }

    portoServidor = (short) strtol(argv[2], NULL, 10);

    if (portoServidor <= 0) {
        printErro(quemSouEu, "main", "erro nos portos");
        return ERRO;
    }

    snprintf(ipServidor, INET_ADDRSTRLEN, "%s", argv[1]);

    enderecoReceber = NULL;
    enderecoEnviar = NULL;
    enderecoEscuta = NULL;
    fdSocketEscuta = 0;

    signal(SIGINT, sigintHandlerMain);

    fatorWhileLogin = TRUE;
    while (fatorWhileLogin) {
        // FAZ LOGIN
        if ((resultado = doLogin()) == SUCESSO_VOLTAR) {
            signal(SIGINT, SIG_IGN);        // ignorar signal
            signal(SIGUSR1, SIG_IGN);       // ignorar signal
            fatorWhileServico = FALSE;
            fatorWhileLogin = FALSE;
            terminar();

        } else if (resultado == SUCESSO_PROSSEGUIR) {
            fatorWhileServico = TRUE;

        } else {    // ERRO
            signal(SIGINT, SIG_IGN);
            printErro(quemSouEu, "main", "doLogin() == ERRO");
            terminar();
            fatorWhileLogin = FALSE;
            fatorWhileServico = FALSE;
        }

        while (fatorWhileServico) {
            // ESCOLHA DO SERVICO + CONFIGURACAO
            if ((resultado = doServico()) == SUCESSO_VOLTAR) {
                fatorWhileServico = FALSE;
                fatorIniciaServico = FALSE;

            } else if (resultado == SUCESSO_PROSSEGUIR) {
                fatorIniciaServico = TRUE;

            } else {    // ERRO
                signal(SIGINT, SIG_IGN);
                printErro(quemSouEu, "main", "doServico() == ERRO");
                terminar();
                fatorWhileLogin = FALSE;
                fatorWhileServico = FALSE;
                fatorIniciaServico = FALSE;
            }

            // INICIA SERVICO
            if (fatorIniciaServico) {
                inet_ntop(AF_INET, &(enderecoEscuta->sin_addr.s_addr), printEnderecoEscuta, INET_ADDRSTRLEN);
                inet_ntop(AF_INET, &(enderecoEnviar->sin_addr.s_addr), printEnderecoEnviar, INET_ADDRSTRLEN);
                putchar('\n');
                printInfo(quemSouEu,"----- CHAT -----");
                snprintf(strTemp, SIZE_STR_TEMP, "RECEBE EM:  ID=\"%s\"  IP=\"%s:%d\"  |  ENVIA PARA:  ID=\"%s\"  IP=\"%s:%d\"", username, printEnderecoEscuta, ntohs(enderecoEscuta->sin_port), usernameDestino, printEnderecoEnviar, ntohs(enderecoEnviar->sin_port));
                printInfo(quemSouEu, strTemp);

                signal(SIGINT, SIG_IGN);        // ignorar signal
                signal(SIGUSR1, SIG_IGN);       // ignorar signal
                if ((pidFilho = fork()) == 0) {
                    // FILHO (pid == 0)
                    snprintf(quemSouEu, SIZE_STR_QUEM, CLIENTE_RECETOR);
                    signal(SIGUSR1, sigusr1Handler);

                    processoRecetorUDP();
                    signal(SIGUSR1, SIG_IGN);
                    kill(getppid(), SIGINT);
                    terminar();
                    fatorWhileServico = FALSE;
                    fatorWhileLogin = FALSE;
                    // TERMINA SEMPRE

                } else {
                    // PAI CONTINUA...
                    snprintf(quemSouEu, SIZE_STR_QUEM, CLIENTE_EMISSOR);
                    signal(SIGINT, sigintHandler);

                    if (processoEmissorUDP() == ERRO) {
                        signal(SIGINT, SIG_IGN);
                        kill(pidFilho, SIGUSR1);
                        terminar();
                        waitpid(pidFilho, NULL, 0);
                        fatorWhileServico = FALSE;
                        fatorWhileLogin = FALSE;

                    } else {    // SUCESSO
                        // VOLTA PRA ESCOLHA DO SERVICO
                        signal(SIGINT, SIG_IGN);        // ignorar signal
                        signal(SIGUSR1, SIG_IGN);       // ignorar signal
                        kill(pidFilho, SIGUSR1);
                        waitpid(pidFilho, NULL, 0);
                        snprintf(quemSouEu, SIZE_STR_QUEM, CLIENTE);
                        signal(SIGINT, sigintHandlerMain);
                        fatorWhileServico = TRUE;
                        fatorWhileLogin = TRUE;
                    }
                }
            }
        }
    }

    printInfo(quemSouEu, "Terminado!");
    exit(0);
}



void sigintHandlerMain(int signum) {
    // Pre-Processos
    if (signum == SIGINT) {
        putchar('\n');
        printInfo(quemSouEu, "SIGINT Recebido!");
        signal(SIGINT, SIG_IGN);    // ignorar signal

        terminar();

        printInfo(quemSouEu, "Processo Principal Terminado!");
        exit(0);
    }
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
    if (enderecoReceber != NULL) {
        free(enderecoReceber);
        enderecoReceber = NULL;
    }

    if (enderecoEnviar != NULL) {
        free(enderecoEnviar);
        enderecoEnviar = NULL;
    }

    if (enderecoEscuta != NULL) {
        free(enderecoEscuta);
        enderecoEscuta = NULL;
    }

    if (fdSocketEscuta > 0) {
        close(fdSocketEscuta);
        fdSocketEscuta = 0;
    }

    printInfo(quemSouEu, "File descriptor (Sockets) fechados; e memoria alocada (Malloc) libertada");
}



type_resultado processoRecetorUDP(void) {
    // FILHO
    char * mensagem = NULL;
    char * idFonte = NULL;
    char * idDestino = NULL;
    uint16_t porto;
    char printEndereco[INET_ADDRSTRLEN];
    int enderecoSize = sizeof(struct sockaddr_in);
    char pacote[SIZE_STR_PACOTE];
    ssize_t nRead;
    ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY];
    char * valorTemp = NULL;
    char strTemp[SIZE_STR_TEMP];

    // TODO ignorar as proprias msgs ???

    while (TRUE) {              // Fica a espera de mensagem
        if ((nRead = recvfrom(fdSocketEscuta, pacote, SIZE_STR_PACOTE, 0, (struct sockaddr *) enderecoReceber, (socklen_t *)&enderecoSize)) <= 0) {
            printErro(quemSouEu, "processoRecetorUDP", "Erro inesperado no recvfrom(), nRead <= 0");
            return ERRO;
        }

        pacote[nRead] = '\0';
        valorTemp = NULL;

        if (getListaChaveValorFromPacote(pacote, listaChaveValor) == ERRO) {
            printErro(quemSouEu, "processoRecetorUDP", "Pacote com erros. (ignorado)");

        } else {
            valorTemp = getValorByChaveFromLista(CHAVE_TYPE, listaChaveValor);

            if (valorTemp != NULL && strcmp(valorTemp, VALOR_ENVIAR_MSG) == 0) {
                idFonte = getValorByChaveFromLista(CHAVE_ID_FONTE, listaChaveValor);
                idDestino = getValorByChaveFromLista(CHAVE_ID_DESTINO, listaChaveValor);
                mensagem = getValorByChaveFromLista(CHAVE_MSG, listaChaveValor);
                inet_ntop(AF_INET, &(enderecoReceber->sin_addr.s_addr), printEndereco, INET_ADDRSTRLEN);
                porto = ntohs(enderecoReceber->sin_port);

                printMsgChat(idFonte, idDestino, printEndereco, porto, mensagem);

            } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_ERRO) == 0) {
                valorTemp = getValorByChaveFromLista(CHAVE_MSG, listaChaveValor);
                if (valorTemp == NULL) {
                    valorTemp = "Sem Msg.";
                }
                snprintf(strTemp, SIZE_STR_TEMP, "Ocorreu um erro! (Msg.: \"%s\"). (ignorado)", valorTemp);
                printInfo(QUEM_ANONIMO, strTemp);

            } else {
                printErro(quemSouEu, "processoRecetorUDP", "Pacote inesperado / com erros. (ignorado)");
            }
        }
    }
}



type_resultado processoEmissorUDP(void) {
    // PAI
    char mensagem[SIZE_STR_VALOR];
    int enderecoSize = sizeof(struct sockaddr_in);
    char pacote[SIZE_STR_PACOTE];

    while (TRUE) {                                                              // Fica a espera do input do utilizador
        getStringStdin(mensagem, SIZE_STR_VALOR);                               // Pega o input do utilizador
        if (mensagem[0] != '\0') {                                              // se estiver vazia, ignora
            if (strcmp(mensagem, "QUIT") == 0) {                                // permite sair
                printInfo(quemSouEu, "A encerrar o chat...");
                return SUCESSO;
            } else {
                verificaAndCorrigeMsg(mensagem, SIZE_STR_VALOR);                // verifica pra nao ter chars proibidos (evita bugs)
                inicializarPacote(pacote);                                      // Cria um pacote para enviar ao servidor
                insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_ENVIAR_MSG);
                insertChaveValorIntoPacote(pacote, CHAVE_ID_FONTE, username);
                insertChaveValorIntoPacote(pacote, CHAVE_ID_DESTINO, usernameDestino);
                insertChaveValorIntoPacote(pacote, CHAVE_MSG, mensagem);

                if ( sendto(fdSocketEscuta, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoEnviar, enderecoSize) <= 0) {    // Enviar a mensagem
                    printErro(quemSouEu, "processoEmissorUDP", "Erro inesperado no sendto(), nSend <= 0");
                    return ERRO;
                }
            }
        }
    }
}



type_resultado doLogin(void) {
    int enderecoSize = sizeof(struct sockaddr_in);
    char pacote[SIZE_STR_PACOTE];
    char password[SIZE_STR_VALOR_LITE];
    int opcaoMenu;
    type_boolean fatorWhile;
    char inputStr[SIZE_STR_VALOR_LITE];
    ssize_t nRead;
    ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY];
    char * valorTemp = NULL;
    char strTemp[SIZE_STR_TEMP];

    if (conectaServidor() == ERRO) {
        printErro(quemSouEu, "doLogin", "Nao foi possivel conectar ao servidor, conectaServidor() == ERRO");
        return ERRO;
    }

    putchar('\n');
    printInfo(quemSouEu,"----- MENU LOGIN -----");
    fatorWhile = TRUE;
    while (fatorWhile) {
        do {
            printf("Escolha uma opcao (numero):\n");
            printf("1. Fazer Login.\n");
            printf("2. Sair do Programa.\n> ");
            getStringStdin(inputStr, SIZE_STR_VALOR);
            opcaoMenu = (int) strtol(inputStr, NULL, 10);
            if (opcaoMenu != 1 && opcaoMenu != 2) {
                printf("Opcao Errada! Tente Novamente!\n");
            }
        } while (opcaoMenu != 1 && opcaoMenu != 2);

        if (opcaoMenu == 1) {
            do {
                printf("Introduza o username:\n> ");
                getStringStdin(inputStr, SIZE_STR_VALOR_LITE);
                if (inputStr[0] == '\0' || verificaMsg(inputStr, SIZE_STR_VALOR_LITE) == TRUE) {
                    printf("Username invalido! Tente Novamente!\n");
                    inputStr[0] = '\0';
                }
            } while (inputStr[0] == '\0');
            snprintf(username, SIZE_STR_VALOR_LITE, "%s", inputStr);

            do {
                printf("Introduza a password:\n> ");
                getStringStdin(inputStr, SIZE_STR_VALOR);
                if (inputStr[0] == '\0' || verificaMsg(inputStr, SIZE_STR_VALOR_LITE) == TRUE) {
                    printf("Password invalida! Tente Novamente!\n");
                    inputStr[0] = '\0';
                }
            } while (inputStr[0] == '\0');
            snprintf(password, SIZE_STR_VALOR_LITE, "%s", inputStr);

            inicializarPacote(pacote);
            insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_LOGIN);
            insertChaveValorIntoPacote(pacote, CHAVE_USERNAME, username);
            insertChaveValorIntoPacote(pacote, CHAVE_PASSWORD, password);

            if ( sendto(fdSocketEscuta, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoEnviar, enderecoSize) <= 0) {
                printErro(quemSouEu, "doLogin", "Erro inesperado no sendto(), nSend <= 0");
                return ERRO;
            }

            if ((nRead = recvfrom(fdSocketEscuta, pacote, SIZE_STR_PACOTE, 0, (struct sockaddr *) enderecoReceber, (socklen_t *)&enderecoSize)) <= 0) {
                printErro(quemSouEu, "doLogin", "Erro inesperado no recvfrom(), nRead <= 0");
                return ERRO;
            }

            pacote[nRead] = '\0';
            valorTemp = NULL;

            if (getListaChaveValorFromPacote(pacote, listaChaveValor) == ERRO) {
                printErro(quemSouEu, "doLogin", "Pacote com erros. (ignorado)");
                printInfo(quemSouEu, "Tente Novamente!");

            } else {
                valorTemp = getValorByChaveFromLista(CHAVE_TYPE, listaChaveValor);

                if (valorTemp != NULL && strcmp(valorTemp, VALOR_STATUS) == 0) {
                    valorTemp = getValorByChaveFromLista(CHAVE_LOGGED, listaChaveValor);

                    if (valorTemp != NULL && strcmp(valorTemp, VALOR_TRUE) == 0) {
                        printInfo(SERVIDOR, "Login efetuado com sucesso!");
                        fatorWhile = FALSE;

                    } else {
                        valorTemp = getValorByChaveFromLista(CHAVE_MSG, listaChaveValor);
                        if (valorTemp == NULL) {
                            valorTemp = "Sem Msg.";
                        }
                        snprintf(strTemp, SIZE_STR_TEMP, "Nao foi possivel fazer login! (Msg.: \"%s\")", valorTemp);
                        printInfo(SERVIDOR, strTemp);
                        printInfo(quemSouEu, "Tente Novamente!");
                    }

                } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_ERRO) == 0) {
                    valorTemp = getValorByChaveFromLista(CHAVE_MSG, listaChaveValor);
                    if (valorTemp == NULL) {
                        valorTemp = "Sem Msg.";
                    }
                    snprintf(strTemp, SIZE_STR_TEMP, "Ocorreu um erro no servidor ao fazer login! (Msg.: \"%s\")", valorTemp);
                    printInfo(SERVIDOR, strTemp);
                    printInfo(quemSouEu, "Tente Novamente!");

                } else {
                    printErro(quemSouEu, "doLogin", "Pacote inesperado / com erros. (ignorado)");
                    printInfo(quemSouEu, "Tente Novamente!");
                }
            }

        } else {
            fatorWhile = FALSE;
        }
    }

    if (opcaoMenu == 1) {
        return SUCESSO_PROSSEGUIR;
    }

    return SUCESSO_VOLTAR;
}



type_resultado doServico(void) {
    int enderecoSize = sizeof(struct sockaddr_in);
    char pacote[SIZE_STR_PACOTE];
    int opcaoMenu;
    type_boolean fatorWhile;
    char inputStr[SIZE_STR_VALOR_LITE];
    ssize_t nRead;
    ChaveValorRecord listaChaveValor[NUM_MAX_PARES_ARRAY];
    char * valorTemp = NULL;
    char strTemp[SIZE_STR_TEMP];
    Servico servico;

    if (conectaServidor() == ERRO) {
        printErro(quemSouEu, "doServico", "Nao foi possivel conectar ao servidor, conectaServidor() == ERRO");
        return ERRO;
    }

    inicializarPacote(pacote);
    insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_GET_SERVICO);
    insertChaveValorIntoPacote(pacote, CHAVE_USERNAME, username);

    if ( sendto(fdSocketEscuta, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoEnviar, enderecoSize) <= 0) {
        printErro(quemSouEu, "doServico", "Erro inesperado no sendto(), nSend <= 0");
        return ERRO;
    }

    if ((nRead = recvfrom(fdSocketEscuta, pacote, SIZE_STR_PACOTE, 0, (struct sockaddr *) enderecoReceber, (socklen_t *)&enderecoSize)) <= 0) {
        printErro(quemSouEu, "doServico", "Erro inesperado no recvfrom(), nRead <= 0");
        return ERRO;
    }

    pacote[nRead] = '\0';
    valorTemp = NULL;

    if (getListaChaveValorFromPacote(pacote, listaChaveValor) == ERRO) {
        printErro(quemSouEu, "doServico", "Pacote com erros.");
        printInfo(quemSouEu, "Sessao Terminada! A voltar ao Menu Login...");
        return SUCESSO_VOLTAR;

    } else {
        valorTemp = getValorByChaveFromLista(CHAVE_TYPE, listaChaveValor);

        if (valorTemp != NULL && strcmp(valorTemp, VALOR_SERVICO_AUTORIZADO) == 0) {
            valorTemp = getValorByChaveFromLista(CHAVE_VALOR_CLIENTE_SERVIDOR, listaChaveValor);
            if (valorTemp != NULL && strcmp(valorTemp, VALOR_TRUE) == 0) {
                servico.clienteServidor = TRUE;

            } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_FALSE) == 0) {
                servico.clienteServidor = FALSE;

            } else {
                printErro(quemSouEu, "doServico", "Pacote com erros");
                printInfo(quemSouEu, "Sessao Terminada! A voltar ao Menu Login...");
                return SUCESSO_VOLTAR;
            }

            valorTemp = getValorByChaveFromLista(CHAVE_VALOR_P2P, listaChaveValor);
            if (valorTemp != NULL && strcmp(valorTemp, VALOR_TRUE) == 0) {
                servico.p2p = TRUE;

            } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_FALSE) == 0) {
                servico.p2p = FALSE;

            } else {
                printErro(quemSouEu, "doServico", "Pacote com erros");
                printInfo(quemSouEu, "Sessao Terminada! A voltar ao Menu Login...");
                return SUCESSO_VOLTAR;
            }

            valorTemp = getValorByChaveFromLista(CHAVE_VALOR_MULTICAST, listaChaveValor);
            if (valorTemp != NULL && strcmp(valorTemp, VALOR_TRUE) == 0) {
                servico.multicast = TRUE;

            } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_FALSE) == 0) {
                servico.multicast = FALSE;

            } else {
                printErro(quemSouEu, "doServico", "Pacote com erros");
                printInfo(quemSouEu, "Sessao Terminada! A voltar ao Menu Login...");
                return SUCESSO_VOLTAR;
            }

        } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_ERRO) == 0) {
            valorTemp = getValorByChaveFromLista(CHAVE_MSG, listaChaveValor);
            if (valorTemp == NULL) {
                valorTemp = "Sem Msg.";
            }
            snprintf(strTemp, SIZE_STR_TEMP, "Ocorreu um erro no servidor ao pedir servicos autorizados! (Msg.: \"%s\")", valorTemp);
            printInfo(SERVIDOR, strTemp);
            printInfo(quemSouEu, "Sessao Terminada! A voltar ao Menu Login...");
            return SUCESSO_VOLTAR;

        } else {
            printErro(quemSouEu, "doServico", "Pacote inesperado / com erros");
            printInfo(quemSouEu, "Sessao Terminada! A voltar ao Menu Login...");
            return SUCESSO_VOLTAR;
        }
    }

    putchar('\n');
    printInfo(quemSouEu,"----- MENU SERVICO -----");
    fatorWhile = TRUE;
    while (fatorWhile) {
        do {
            printf("Escolha um servico disponivel (opcao/numero):\n");
            if (servico.clienteServidor == TRUE)
                printf("1. Cliente-Servidor.\n");
            if (servico.p2p == TRUE)
                printf("2. P2P.\n");
            if (servico.multicast == TRUE)
                printf("3. Grupo Multicast.\n");
            printf("4. Loggout (Voltar ao Login).\n> ");
            getStringStdin(inputStr, SIZE_STR_VALOR);
            opcaoMenu = (int) strtol(inputStr, NULL, 10);
            if (opcaoMenu > 4 || opcaoMenu <= 0 || (opcaoMenu == 1 && servico.clienteServidor == FALSE) || (opcaoMenu == 2 && servico.p2p == FALSE) || (opcaoMenu == 3 && servico.multicast == FALSE)) {
                printf("Opcao Errada! Tente Novamente!\n");
                opcaoMenu = 0;
            }
        } while (opcaoMenu > 4 || opcaoMenu <= 0);

        switch (opcaoMenu) {
            case 1:     // Cliente-Servidor
                do {
                    printf("Introduza o username/ID de destino:\n> ");
                    getStringStdin(inputStr, SIZE_STR_VALOR_LITE);
                    if (inputStr[0] == '\0' || verificaMsg(inputStr, SIZE_STR_VALOR_LITE) == TRUE) {
                        printf("Username invalido! Tente Novamente!\n");
                        inputStr[0] = '\0';
                    }
                } while (inputStr[0] == '\0');
                snprintf(usernameDestino, SIZE_STR_VALOR_LITE, "%s", inputStr);

                inicializarPacote(pacote);
                insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_SERVICO_ESCOLHIDO);
                insertChaveValorIntoPacote(pacote, CHAVE_SERVICO, CHAVE_VALOR_CLIENTE_SERVIDOR);
                insertChaveValorIntoPacote(pacote, CHAVE_ID, usernameDestino);

                if ( sendto(fdSocketEscuta, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoEnviar, enderecoSize) <= 0) {
                    printErro(quemSouEu, "doServico", "Erro inesperado no sendto(), nSend <= 0");
                    return ERRO;
                }

                if ((nRead = recvfrom(fdSocketEscuta, pacote, SIZE_STR_PACOTE, 0, (struct sockaddr *) enderecoReceber, (socklen_t *)&enderecoSize)) <= 0) {
                    printErro(quemSouEu, "doServico", "Erro inesperado no recvfrom(), nRead <= 0");
                    return ERRO;
                }

                pacote[nRead] = '\0';
                valorTemp = NULL;

                if (getListaChaveValorFromPacote(pacote, listaChaveValor) == ERRO) {
                    printErro(quemSouEu, "doServico", "Pacote com erros. (ignorado)");
                    printInfo(quemSouEu, "Tente Novamente!");

                } else {
                    valorTemp = getValorByChaveFromLista(CHAVE_TYPE, listaChaveValor);

                    if (valorTemp != NULL && strcmp(valorTemp, CHAVE_VALOR_CLIENTE_SERVIDOR) == 0) {
                        valorTemp = getValorByChaveFromLista(CHAVE_IS_VALIDO, listaChaveValor);

                        if (valorTemp != NULL && strcmp(valorTemp, VALOR_TRUE) == 0) {
                            printInfo(SERVIDOR, "Servico ativado com sucesso!");
                            fatorWhile = FALSE;

                        } else {
                            printInfo(SERVIDOR, "Nao foi possivel ativar o servico!");
                            printInfo(quemSouEu, "Verifique os dados. Tente Novamente!");
                        }

                    } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_ERRO) == 0) {
                        valorTemp = getValorByChaveFromLista(CHAVE_MSG, listaChaveValor);
                        if (valorTemp == NULL) {
                            valorTemp = "Sem Msg.";
                        }
                        snprintf(strTemp, SIZE_STR_TEMP, "Ocorreu um erro no servidor ao selecionar o servico escolhido! (Msg.: \"%s\")", valorTemp);
                        printInfo(SERVIDOR, strTemp);
                        printInfo(quemSouEu, "Verifique os dados. Tente Novamente!");

                    } else {
                        printErro(quemSouEu, "doServico", "Pacote inesperado / com erros. (ignorado)");
                        printInfo(quemSouEu, "Tente Novamente!");
                    }
                }
                break;

            case 2:     // P2P
                do {
                    printf("Introduza o username/ID de destino:\n> ");
                    getStringStdin(inputStr, SIZE_STR_VALOR_LITE);
                    if (inputStr[0] == '\0' || verificaMsg(inputStr, SIZE_STR_VALOR_LITE) == TRUE) {
                        printf("Username invalido! Tente Novamente!\n");
                        inputStr[0] = '\0';
                    }
                } while (inputStr[0] == '\0');
                snprintf(usernameDestino, SIZE_STR_VALOR_LITE, "%s", inputStr);

                inicializarPacote(pacote);
                insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_SERVICO_ESCOLHIDO);
                insertChaveValorIntoPacote(pacote, CHAVE_SERVICO, CHAVE_VALOR_P2P);
                insertChaveValorIntoPacote(pacote, CHAVE_ID, usernameDestino);

                if ( sendto(fdSocketEscuta, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoEnviar, enderecoSize) <= 0) {
                    printErro(quemSouEu, "doServico", "Erro inesperado no sendto(), nSend <= 0");
                    return ERRO;
                }

                if ((nRead = recvfrom(fdSocketEscuta, pacote, SIZE_STR_PACOTE, 0, (struct sockaddr *) enderecoReceber, (socklen_t *)&enderecoSize)) <= 0) {
                    printErro(quemSouEu, "doServico", "Erro inesperado no recvfrom(), nRead <= 0");
                    return ERRO;
                }

                pacote[nRead] = '\0';
                valorTemp = NULL;

                if (getListaChaveValorFromPacote(pacote, listaChaveValor) == ERRO) {
                    printErro(quemSouEu, "doServico", "Pacote com erros. (ignorado)");
                    printInfo(quemSouEu, "Tente Novamente!");

                } else {
                    valorTemp = getValorByChaveFromLista(CHAVE_TYPE, listaChaveValor);

                    if (valorTemp != NULL && strcmp(valorTemp, CHAVE_VALOR_P2P) == 0) {
                        valorTemp = getValorByChaveFromLista(CHAVE_IP, listaChaveValor);

                        if (valorTemp != NULL) {
                            snprintf(ipDestino, INET_ADDRSTRLEN, "%s", valorTemp);
                            if (conectaP2p() == ERRO) {
                                printErro(quemSouEu, "doServico", "Nao foi possivel conectar ao P2P, conectaP2p() == ERRO");
                                return ERRO;
                            }
                            printInfo(SERVIDOR, "Servico ativado com sucesso!");
                            fatorWhile = FALSE;

                        } else {
                            printInfo(SERVIDOR, "Nao foi possivel ativar o servico!");
                            printInfo(quemSouEu, "Verifique os dados. Tente Novamente!");
                        }

                    } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_ERRO) == 0) {
                        valorTemp = getValorByChaveFromLista(CHAVE_MSG, listaChaveValor);
                        if (valorTemp == NULL) {
                            valorTemp = "Sem Msg.";
                        }
                        snprintf(strTemp, SIZE_STR_TEMP, "Ocorreu um erro no servidor ao selecionar o servico escolhido! (Msg.: \"%s\")", valorTemp);
                        printInfo(SERVIDOR, strTemp);
                        printInfo(quemSouEu, "Verifique os dados. Tente Novamente!");

                    } else {
                        printErro(quemSouEu, "doServico", "Pacote inesperado / com erros. (ignorado)");
                        printInfo(quemSouEu, "Tente Novamente!");
                    }
                }
                break;

            case 3:     // Grupo Multicast
                do {
                    printf("Introduza o nome/ID do grupo (se nao existir sera criado um novo):\n> ");
                    getStringStdin(inputStr, SIZE_STR_VALOR_LITE);
                    if (inputStr[0] == '\0' || verificaMsg(inputStr, SIZE_STR_VALOR_LITE) == TRUE) {
                        printf("Username invalido! Tente Novamente!\n");
                        inputStr[0] = '\0';
                    }
                } while (inputStr[0] == '\0');
                snprintf(usernameDestino, SIZE_STR_VALOR_LITE, "%s", inputStr);

                inicializarPacote(pacote);
                insertChaveValorIntoPacote(pacote, CHAVE_TYPE, VALOR_SERVICO_ESCOLHIDO);
                insertChaveValorIntoPacote(pacote, CHAVE_SERVICO, CHAVE_VALOR_MULTICAST);
                insertChaveValorIntoPacote(pacote, CHAVE_ID, usernameDestino);

                if ( sendto(fdSocketEscuta, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) enderecoEnviar, enderecoSize) <= 0) {
                    printErro(quemSouEu, "doServico", "Erro inesperado no sendto(), nSend <= 0");
                    return ERRO;
                }

                if ((nRead = recvfrom(fdSocketEscuta, pacote, SIZE_STR_PACOTE, 0, (struct sockaddr *) enderecoReceber, (socklen_t *)&enderecoSize)) <= 0) {
                    printErro(quemSouEu, "doServico", "Erro inesperado no recvfrom(), nRead <= 0");
                    return ERRO;
                }

                pacote[nRead] = '\0';
                valorTemp = NULL;

                if (getListaChaveValorFromPacote(pacote, listaChaveValor) == ERRO) {
                    printErro(quemSouEu, "doServico", "Pacote com erros. (ignorado)");
                    printInfo(quemSouEu, "Tente Novamente!");

                } else {
                    valorTemp = getValorByChaveFromLista(CHAVE_TYPE, listaChaveValor);

                    if (valorTemp != NULL && strcmp(valorTemp, CHAVE_VALOR_MULTICAST) == 0) {
                        valorTemp = getValorByChaveFromLista(CHAVE_IP, listaChaveValor);

                        if (valorTemp != NULL) {
                            snprintf(ipDestino, INET_ADDRSTRLEN, "%s", valorTemp);
                            if (conectaMulticast() == ERRO) {
                                printErro(quemSouEu, "doServico", "Nao foi possivel conectar ao Multicast, conectaMulticast() == ERRO");
                                return ERRO;
                            }
                            printInfo(SERVIDOR, "Servico ativado com sucesso!");
                            fatorWhile = FALSE;

                        } else {
                            printInfo(SERVIDOR, "Nao foi possivel ativar o servico!");
                            printInfo(quemSouEu, "Verifique os dados. Tente Novamente!");
                        }

                    } else if (valorTemp != NULL && strcmp(valorTemp, VALOR_ERRO) == 0) {
                        valorTemp = getValorByChaveFromLista(CHAVE_MSG, listaChaveValor);
                        if (valorTemp == NULL) {
                            valorTemp = "Sem Msg.";
                        }
                        snprintf(strTemp, SIZE_STR_TEMP, "Ocorreu um erro no servidor ao selecionar o servico escolhido! (Msg.: \"%s\")", valorTemp);
                        printInfo(SERVIDOR, strTemp);
                        printInfo(quemSouEu, "Verifique os dados. Tente Novamente!");

                    } else {
                        printErro(quemSouEu, "doServico", "Pacote inesperado / com erros. (ignorado)");
                        printInfo(quemSouEu, "Tente Novamente!");
                    }
                }
                break;

            case 4:     // SAIR
            default:
                fatorWhile = FALSE;
                break;
        }
    }

    if (opcaoMenu != 4) {
        return SUCESSO_PROSSEGUIR;
    }

    return SUCESSO_VOLTAR;  // == 4
}



type_resultado conectaServidor (void) {
    struct hostent * ptrIp = NULL;

    if ( conectaLimpar() == ERRO) {
        printErro(quemSouEu, "conectaServidor", "Nao foi possivel fechar as conexoes existentes");
        return ERRO;
    }

    if ((ptrIp = gethostbyname(ipServidor)) == 0) {
        printErro(quemSouEu, "conectaServidor", "gethostbyname() == NULL, impossivel obter endereco");
        return ERRO;
    }

    enderecoEscuta->sin_family = AF_INET;
    enderecoEscuta->sin_addr.s_addr = htonl(INADDR_ANY);
    enderecoEscuta->sin_port = htons(PORTO_CLIENTE);

    enderecoEnviar->sin_family = AF_INET;
    enderecoEnviar->sin_addr.s_addr = ((struct in_addr *)(ptrIp->h_addr))->s_addr;
    enderecoEnviar->sin_port = htons(portoServidor);

    if ((fdSocketEscuta = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printErro(quemSouEu, "conectaServidor", "Socket < 0");
        return ERRO;
    }

    if (bind(fdSocketEscuta, (struct sockaddr*) enderecoEscuta, sizeof(struct sockaddr_in)) < 0) {
        printErro(quemSouEu, "conectaServidor", "bind < 0");
        return ERRO;
    }

    return SUCESSO;
}



type_resultado conectaP2p (void) {
    struct hostent * ptrIp = NULL;

    if ( conectaLimpar() == ERRO) {
        printErro(quemSouEu, "conectaP2p", "Nao foi possivel fechar as conexoes existentes");
        return ERRO;
    }

    if ((ptrIp = gethostbyname(ipDestino)) == 0) {
        printErro(quemSouEu, "conectaP2p", "gethostbyname() == NULL, impossivel obter endereco");
        return ERRO;
    }

    enderecoEscuta->sin_family = AF_INET;
    enderecoEscuta->sin_addr.s_addr = htonl(INADDR_ANY);
    enderecoEscuta->sin_port = htons(PORTO_CLIENTE);

    enderecoEnviar->sin_family = AF_INET;
    enderecoEnviar->sin_addr.s_addr = ((struct in_addr *)(ptrIp->h_addr))->s_addr;
    enderecoEnviar->sin_port = htons(PORTO_CLIENTE);

    if ((fdSocketEscuta = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printErro(quemSouEu, "conectaP2p", "Socket < 0");
        return ERRO;
    }

    if (bind(fdSocketEscuta, (struct sockaddr*) enderecoEscuta, sizeof(struct sockaddr_in)) < 0) {
        printErro(quemSouEu, "conectaP2p", "bind < 0");
        return ERRO;
    }

    return SUCESSO;
}



type_resultado conectaMulticast (void) {
    struct hostent * ptrIp = NULL;
    struct ip_mreq enderecoMulticast;
    int multicastTTL = 3;
    int soReuseAddr = 1;
    int ipMulticastLoop = 0;

    if (conectaLimpar() == ERRO) {
        printErro(quemSouEu, "conectaMulticast", "Nao foi possivel fechar as conexoes existentes");
        return ERRO;
    }

    if ((ptrIp = gethostbyname(ipDestino)) == 0) {
        printErro(quemSouEu, "conectaMulticast", "gethostbyname() == NULL, impossivel obter endereco");
        return ERRO;
    }

    enderecoEscuta->sin_family = AF_INET;
    enderecoEscuta->sin_addr.s_addr = htonl(INADDR_ANY);
    enderecoEscuta->sin_port = htons(PORTO_CLIENTE);

    enderecoReceber->sin_family = AF_INET;
    enderecoReceber->sin_addr.s_addr = htonl(INADDR_ANY);
    enderecoReceber->sin_port = htons(PORTO_CLIENTE);

    enderecoEnviar->sin_family = AF_INET;
    enderecoEnviar->sin_addr.s_addr = ((struct in_addr *)(ptrIp->h_addr))->s_addr;
    enderecoEnviar->sin_port = htons(PORTO_CLIENTE);

    if ((fdSocketEscuta = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printErro(quemSouEu, "conectaMulticast", "Socket < 0");
        return ERRO;
    }

    if (setsockopt(fdSocketEscuta, IPPROTO_IP, SO_REUSEADDR, (void *) &soReuseAddr, sizeof(soReuseAddr)) < 0) {
        printErro(quemSouEu, "conectaMulticast", "setsockopt < 0, SO_REUSEADDR");
        return ERRO;
    }

    if (bind(fdSocketEscuta, (struct sockaddr*) enderecoEscuta, sizeof(struct sockaddr_in)) < 0) {
        printErro(quemSouEu, "conectaMulticast", "bind < 0");
        return ERRO;
    }

    enderecoMulticast.imr_multiaddr.s_addr = inet_addr(ipDestino);
    enderecoMulticast.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(fdSocketEscuta, IPPROTO_IP, IP_ADD_MEMBERSHIP, &enderecoMulticast, sizeof(enderecoMulticast)) < 0) {
        printErro(quemSouEu, "conectaMulticast", "setsockopt < 0, IP_ADD_MEMBERSHIP");
        return ERRO;
    }

    if (setsockopt(fdSocketEscuta, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &multicastTTL, sizeof(multicastTTL)) < 0) {
        printErro(quemSouEu, "conectaMulticast", "setsockopt < 0, IP_MULTICAST_TTL");
        return ERRO;
    }

    if (setsockopt(fdSocketEscuta, IPPROTO_IP, IP_MULTICAST_LOOP, (void *) &ipMulticastLoop, sizeof(ipMulticastLoop)) < 0) {
        printErro(quemSouEu, "conectaMulticast", "setsockopt < 0, IP_MULTICAST_LOOP");
        return ERRO;
    }

    return SUCESSO;
}



type_resultado conectaLimpar (void) {
    if (enderecoEscuta != NULL) {
        free(enderecoEscuta);
        enderecoEscuta = NULL;
    }

    if (enderecoEnviar != NULL) {
        free(enderecoEnviar);
        enderecoEnviar = NULL;
    }

    if (enderecoReceber != NULL) {
        free(enderecoReceber);
        enderecoReceber = NULL;
    }

    if (fdSocketEscuta > 0) {
        close(fdSocketEscuta);
        fdSocketEscuta = 0;
    }

    if ((enderecoEscuta = mallocEndereco()) == NULL) {
        printErro(quemSouEu, "conectaLimpar", "mallocEndereco() == NULL");
        return ERRO;
    }

    if ((enderecoEnviar = mallocEndereco()) == NULL) {
        printErro(quemSouEu, "conectaLimpar", "mallocEndereco() == NULL");
        return ERRO;
    }

    if ((enderecoReceber = mallocEndereco()) == NULL) {
        printErro(quemSouEu, "conectaLimpar", "mallocEndereco() == NULL");
        return ERRO;
    }

    return SUCESSO;
}



void printMsgChat (const char * idFonte, const char * idDestino, const char * ip, int porto, const char * msg) {
    if (idFonte == NULL)
        idFonte = "?";
    if (idDestino == NULL)
        idDestino = "?";
    if (ip == NULL)
        ip = "?";
    if (msg == NULL)
        msg = "?";

    printf("DE: \"%s\",  PARA: \"%s\" (%s:%d),  MSG: \"%s\"\n", idFonte, idDestino, ip, porto, msg);
    fflush(stdout);
}


