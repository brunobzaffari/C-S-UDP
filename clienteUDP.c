#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>

/*Arquivo Receb-------------------------------------------------------------------------------------------------------------------------*/
bool segmenta_arq_recebimento(const char *recebeporta4, char *nome_arq, char *ext, char *tam)
{
    if (sscanf(recebeporta4, "%s %s %s", nome_arq, ext, tam) == 3)
    {
        return true;
    }
    return false;
}

// Define a função que salvará o arquivo
bool salva_arquivo(const char conteudo[], const char *nome_arq, const char *caminho, const char *ext)
{
    // Cria o caminho completo para o arquivo
    char caminho_completo[128] = {0};
    sprintf(caminho_completo, "%s/%s.%s", caminho, nome_arq, ext);

    // Abre o arquivo para gravação
    FILE *fp = fopen(caminho_completo, "wb");
    if (fp == NULL)
    {
        perror("Erro ao abrir o arquivo para gravação");
        return false;
    }

    // Grava o conteúdo no arquivo
    size_t len = strlen(conteudo); // Assume que o conteúdo é uma string terminada em nulo
    if (fwrite(conteudo, 1, len, fp) != len)
    {
        perror("Erro ao escrever no arquivo");
        fclose(fp);
        return false;
    }

    // Fecha o arquivo
    fclose(fp);
    return true;
}
/*--------------------------------------------------------------------------------------------------------------------------------------*/
/*Arquivo Envio-------------------------------------------------------------------------------------------------------------------------*/
bool segmnta_arq(const char *txt, char *cod, char *caminho)
{
    if (strncmp(txt, "/arq", 4) == 0)
    {
        strcpy(cod, "/arq"); // Copia "/arq" para 'cod'

        const char *resto = txt + 4;
        if (!isspace(*resto))
        { // Se o próximo caractere não for um espaço, retorne falso
            return false;
        }

        while (isspace(*resto))
        { // Ignorar espaços
            resto++;
        }

        if (*resto == '\0')
        { // Se o resto da string for vazio, retorne falso
            return false;
        }

        strcpy(caminho, resto); // Copia o resto da string, ignorando espaços iniciais

        return true;
    }
    return false;
}

bool le_arq(const char *caminho, char *tam, char *ext, char *arquivo, char *nome_arq)
{
    char x[128];
    strcpy(x, caminho);
    FILE *fp = fopen(x, "r");

    if (fp == NULL)
    {
        perror("Erro ao abrir arquivo");
        return false;
    }

    fseek(fp, 0, SEEK_END);
    long tamanho = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (tamanho >= 1501)
    {
        printf("O arquivo é muito grande para ser transmitido.\n");
        fclose(fp);
        return false;
    }
    snprintf(tam, 5, "%ld", tamanho);

    char *nome = strrchr(caminho, '/');
    if (nome != NULL)
    {
        nome++;
    }
    else
    {
        nome = (char *)caminho;
    }

    char *extensao = strrchr(nome, '.');
    if (extensao != NULL)
    {
        strcpy(ext, extensao + 1);
        strncpy(nome_arq, nome, extensao - nome); // Copia o nome do arquivo sem a extensão
        nome_arq[extensao - nome] = '\0';         // Adicionar o null-terminator manualmente
    }
    else
    {
        strcpy(ext, "");
        strcpy(nome_arq, nome);
    }

    fread(arquivo, 1, tamanho, fp);

    arquivo[tamanho] = '\0';

    fclose(fp);
    return true;
}
/*-----------------------------------------------------------------------------------------------------------------------------------*/

/*Mensagens gerais------------------------------------------------------------------------------------------------------------------------------*/
bool segmenta_geral(char *envioporta, char *txt, char *cod, char *nome, char *mensagem)
{
    if (strncmp(txt, "/lin ", 5) == 0)
    {
        strcpy(cod, "/lin");
    }
    else if ((strncmp(txt, "/sai ", 5) == 0) || (strncmp(txt, "/sai", 5) == 0))
    {
        strcpy(cod, "/sai");
        sprintf(envioporta, "%s", cod);
        return true;
    }
    else if ((strncmp(txt, "/out ", 5) == 0) || (strncmp(txt, "/out", 5) == 0))
    {
        strcpy(cod, "/out");
        sprintf(envioporta, "%s", cod);
        return true;
    }
    else if (strncmp(txt, "/msg ", 5) == 0)
    {
        strcpy(cod, "/msg");
    }
    else if (strncmp(txt, "/mpv ", 5) == 0)
    {
        strcpy(cod, "/mpv");
    }
    else if ((strncmp(txt, "/hlp ", 5) == 0) || (strncmp(txt, "/hlp", 5) == 0))
    {
        strcpy(cod, "/hlp");
        sprintf(envioporta, "%s", cod);
        return true;
    }
    else
    {
        // printf("Mensagem invalida\n");
        return false;
    }
    cod[4] = '\0';

    char *resto = txt + 5;
    if (strcmp(cod, "/lin") == 0)
    {
        sscanf(resto, "%9s", nome);
        if (strlen(nome) > 9)
        {
            perror("Nome grande demais\n");
            return false;
        }
        sprintf(envioporta, "%s %s", cod, nome);
        return true;
    }
    else if (strcmp(cod, "/msg") == 0)
    {
        strncpy(mensagem, resto, 111);
        mensagem[127] = '\0';
        sprintf(envioporta, "%s %s", cod, mensagem);
        return true;
    }
    else if (strcmp(cod, "/mpv") == 0)
    {
        sscanf(resto, "%9s", nome);
        if (strlen(nome) > 9)
        {
            perror("Nome grande demais\n");
            return false;
        }
        resto = strchr(resto, ' ');
        if (resto != NULL)
        {
            strncpy(mensagem, resto + 1, 127);
            mensagem[127] = '\0';
        }
        sprintf(envioporta, "%s %s %s", cod, nome, mensagem);
        return true;
    }
    return false;
}
/*-------------------------------------------------------------------------------------------------------------------------------------*/

int main()
{
    /*Variaveis Uso geral----------------------------------------------------------------------------------------------------------------------------------------------------*/
    struct sockaddr_in server_address7, server_address8;
    struct sockaddr_in client_address5, client_address4;
    char recebeporta4[1501]; // recebimento de mensagens
    char envioporta8[1501];
    char envioporta7[128];
    char mensagem[128];
    char recebeporta5; // controle
    char txt[143];
    char nome[10];
    char cod[5];
    int conectado;
    socklen_t len;
    ssize_t bytes;
    ssize_t x;
    // bool y;

    /*Variaveis para Envio de arquivo-----------------------------------------------------------------------------------------------------------------------------------------*/
    char arquivo[1501];
    char caminho[128];
    char nome_arq[20];
    char ext[10];
    char tam[5];

    /*Variaveis para Recebimento de de arquivo--------------------------------------------------------------------------------------------------------------------------------*/
    char arquivo_in[1501];
    char caminho_in[128] = "/home/brunobavarescozaffari/Desktop/";

    /*------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    /*Criacao de sockets---------------------------------------------------------------------------------------------------------------------------*/
    // Criar o primeiro socket (porta 50004)
    int sock4 = socket(AF_INET, SOCK_DGRAM, 0);
    client_address4.sin_family = AF_INET;
    client_address4.sin_port = htons(50004);
    client_address4.sin_addr.s_addr = INADDR_ANY;
    bind(sock4, (struct sockaddr *)&client_address4, sizeof(client_address4));

    // Criar o segundo socket (porta 50005)
    int sock5 = socket(AF_INET, SOCK_DGRAM, 0);
    client_address5.sin_family = AF_INET;
    client_address5.sin_port = htons(50005);
    client_address5.sin_addr.s_addr = INADDR_ANY;
    bind(sock5, (struct sockaddr *)&client_address5, sizeof(client_address5));

    // Configurar endereço do servidor para porta 50007
    // Porta de entrada
    server_address7.sin_family = AF_INET;
    server_address7.sin_port = htons(50007);
    // server_address7.sin_addr.s_addr = inet_addr("10.0.0.38");
    // server_address7.sin_addr.s_addr = inet_addr("192.168.3.31");
    server_address7.sin_addr.s_addr = inet_addr("192.168.15.30");

    // Configurar endereço do servidor para porta 50008
    server_address8.sin_family = AF_INET;
    server_address8.sin_port = htons(50008);
    // server_address8.sin_addr.s_addr = inet_addr("10.0.0.38");
    // server_address8.sin_addr.s_addr = inet_addr("192.168.3.31");
    server_address8.sin_addr.s_addr = inet_addr("192.168.15.30");

    while (1)
    {

        if (conectado == 0)
        {
            printf("Bem-vindo ao chat! Aqui estão as opções que você pode usar:\n");
            printf("--------------------------------------------------------\n");
            printf("/lin <nikname[9]> : Loga e salva seu nome. Se já estiver logado, informará que você já está logado e não poderá trocar o nome.\n");
            printf("/out : Sai do chat.\n");
            printf("/sai : Sai e encerra o programa.\n");
            printf("--------------------------------------------------------\n");
        }

        while (conectado == 0)
        {
            // Zera tudo
            memset(recebeporta4, 0, sizeof(recebeporta4));
            memset(envioporta7, 0, sizeof(envioporta7));
            memset(nome, 0, sizeof(nome));
            memset(txt, 0, sizeof(txt));
            memset(ext, 0, sizeof(ext));
            memset(tam, 0, sizeof(tam));
            memset(cod, 0, sizeof(cod));
            recebeporta5 = 0;
            x = -2;
            //
            fgets(txt, 143, stdin);
            txt[strlen(txt) - 1] = '\0'; // estava dando mto erro por causa que terminava com o \n(eu acho)
            printf("\n");
            //
            if (segmenta_geral(envioporta7, txt, cod, nome, mensagem))
            { // /lin /out /sai----------------------------------------------------------------------------------------------------------
                if (strcmp(cod, "/lin") == 0)
                { // LIN -----------------------------------------------------------------------------------------------------
                    // Enviar para porta 7
                    sendto(sock4, envioporta7, strlen(envioporta7) + 1, 0, (struct sockaddr *)&server_address7, sizeof(server_address7)); // 4->7
                    len = sizeof(server_address7);
                    x = recvfrom(sock5, &recebeporta5, 1, 0, (struct sockaddr *)&server_address7, &len); // 5<-7
                    if (x > 0)
                    {
                        if (recebeporta5 == '2')
                        { // talvez tirar daqui
                            printf("Agora, voce esta conectado e seu nick es: %s\n", nome);
                            conectado = 1; // Anteriormente/Posteriormente era/vai ser para entrar da thread/processo sender e recive
                        }
                        else if (recebeporta5 == '3')
                        {
                            printf("Servidor cheio, tente novamente mais tarde!\n");
                            conectado = 0; // Anteriormente/Posteriormente era/vai ser para sair da thread/processo sender e recive
                        }
                        else if (recebeporta5 == '4')
                        {
                            // Se ja esta conectado vai receber o nome q esta salvo no servidor
                            len = sizeof(server_address7);
                            x = recvfrom(sock4, recebeporta4, sizeof(recebeporta4) - 1, 0, (struct sockaddr *)&server_address7, &len); // aguarda receber o nome 4<-7
                            recebeporta4[x] = '\0';
                            printf("%s\n", recebeporta4);
                            conectado = 1; // Anteriormente/Posteriormente era/vai ser para entrar da thread/processo sender e recive
                        }
                        else
                        {
                            printf("Erro\n");
                        }
                    }
                    else
                        printf("Erro, ao enviar\n");
                }
    
            else if (strcmp(cod, "/sai") == 0){// SAI -----------------------------------------------------------------------------------------
                    x = sendto(sock4, envioporta7, sizeof(envioporta7), 0, (struct sockaddr *)&server_address7, sizeof(server_address7)); // 4->7
                    if (x > 0){
                        len = sizeof(server_address7);
                        x = recvfrom(sock5, &recebeporta5, 1, 0, (struct sockaddr *)&server_address7, &len); // Controle 5<-7
                        conectado = 0;
                        if (x > 0 && recebeporta5 == '2') printf("Você foi suspenso do chat.\n"); // Controle
                        else if (x > 0 && recebeporta5 == '1') printf("Voce nao estava na lista\n"); // Controle
                        else printf("erro\n");
                        exit(0);
                    }
                    else printf("Erro, ao enviar\n");
                    exit(0);
                }
            else if (strcmp(cod, "/out") == 0){// OUT ---------------------------------------------------------------------------------------
                x = sendto(sock4, envioporta7, sizeof(envioporta7), 0, (struct sockaddr *)&server_address7, sizeof(server_address7)); // 4->7
                if (x > 0)
                {
                    len = sizeof(server_address7);
                    recvfrom(sock5, &recebeporta5, 1, 0, (struct sockaddr *)&server_address7, &len); // Controle 5<-7
                    conectado = 0;
                    if (recebeporta5 == '4')
                        printf("Você foi desconectado do chat.\n"); // Controle
                    else if (recebeporta5 == '1')
                        printf("Você nao esta no chat.\n");
                    else
                        printf("Erro\n");
                }
            else printf("Erro, ao enviar\n");
            }
        else printf("Falha ao processar o texto.\n");
        }
        }

        if (conectado == 1)
        {
            printf("--------------------------------------------------------\n");
            printf("/lin <nikname[9]> : Loga e salva seu nome. Se já estiver logado, informará que você já está logado e não poderá trocar o nome.\n");
            printf("/out : Sai do chat.\n");
            printf("/sai : Sai e encerra o programa.\n");
            printf("/msg <mensagem[128]> : Envia uma mensagem com tamanho de até 127 caracteres.\n");
            printf("/mpv <nikname[9]> <mensagem[127]> : Envia uma mensagem privada com tamanho de até 127 caracteres para o usuário especificado. Se o usuário não existir, você será informado.\n");
            printf("/arq <caminho do arquivo> Envia um arquivo de até 1500 bytes.\n");
            printf("/hlp : Ajuda/Printa opcoes\n");
            printf("--------------------------------------------------------\n");
        }

        /*----------------------------------------------------------------------------------------------------------------------------------------*/
        // Le mensagens-----------------------------------------------------------------------------------------------------------------------------
        while (conectado == 1)
        {

            // Zera tudo
            memset(recebeporta4, 0, sizeof(recebeporta4));
            memset(envioporta8, 0, sizeof(envioporta8));
            memset(arquivo_in, 0, sizeof(arquivo_in));
            memset(mensagem, 0, sizeof(mensagem));
            memset(nome_arq, 0, sizeof(nome_arq));
            memset(caminho, 0, sizeof(caminho));
            memset(arquivo, 0, sizeof(arquivo));
            memset(nome, 0, sizeof(nome));
            memset(txt, 0, sizeof(txt));
            memset(ext, 0, sizeof(ext));
            memset(tam, 0, sizeof(tam));
            memset(cod, 0, sizeof(cod));
            recebeporta5 = 0;
            x = -2;

            //----------------------------------------------------------------------------------------------------------------------------------------------------
            fgets(txt, 143, stdin);
            txt[strlen(txt) - 1] = '\0'; // estava dando mto erro por causa que terminava com o \n(eu acho)
            printf("\n");
            // printf("%s", txt); // deixar debug comentado
            // printf("%ld", strlen(txt));
            // strcpy(txt, "/arq /home/brunobavarescozaffari/Desktop/2.c");
            // strcpy(txt, "/lin bruno");
               // Mensagens gerais----------------------------------------------------------------------------------------------------------------------------
            if (segmenta_geral(envioporta8, txt, cod, nome, mensagem)){ // /lin /out /sai /msg /mpv----------------------------------------------------------------------------------------------------------
                if (strcmp(cod, "/lin") == 0){// LIN -----------------------------------------------------------------------------------------------------
                    sendto(sock4, envioporta8, strlen(envioporta8) + 1, 0, (struct sockaddr *)&server_address8, sizeof(server_address8)); // 4 Enviar para ->8
                    len = sizeof(server_address8);
                    x = recvfrom(sock5, &recebeporta5, 1, 0, (struct sockaddr *)&server_address8, &len); // 5<-8
                    if (x > 0)
                    {
                        if (recebeporta5 == '4')
                        { // Se ja esta conectado vai receber o nome q esta salvo no servidor
                            len = sizeof(server_address8);
                            x = recvfrom(sock4, recebeporta4, sizeof(recebeporta4) - 1, 0, (struct sockaddr *)&server_address8, &len); // aguarda receber o nome 4<-8
                            recebeporta4[x] = '\0';
                            printf("%s\n", recebeporta4);
                            conectado = 1; // Anteriormente/Posteriormente era/vai ser para entrar da thread/processo sender e recive
                        }
                        else if (recebeporta5 == '1')
                        {
                            printf("Voce nao esta na lista!\n");
                            conectado = 0; // Anteriormente/Posteriormente era/vai ser para sair da thread/processo sender e recive
                        }
                        else
                        {
                            printf("Erro\n");
                        }
                    }
                    else
                        printf("Erro, ao enviar!\n");
                }
                else if (strcmp(cod, "/sai") == 0){// SAI -----------------------------------------------------------------------------------------
                    x = sendto(sock4, envioporta8, sizeof(envioporta8), 0, (struct sockaddr *)&server_address7, sizeof(server_address7)); // 4->7
                    if (x > 0)
                    {
                        len = sizeof(server_address8);
                        x = recvfrom(sock5, &recebeporta5, 1, 0, (struct sockaddr *)&server_address7, &len); // Controle 5<-7
                        conectado = 0;
                        if (x > 0 && recebeporta5 == '2')
                        {
                            printf("Você foi suspenso do chat.\n"); // Controle
                            conectado = 0;
                        }
                        else if (x > 0 && recebeporta5 == '1')
                        {
                            printf("Voce nao estava na lista\n"); // Controle
                            conectado = 0;
                        }
                        else
                            printf("erro\n");
                    }
                    else printf("Erro, ao enviar\n");
                    exit(0);
                }
                else if (strcmp(cod, "/out") == 0){// OUT ---------------------------------------------------------------------------------------
                    x = sendto(sock4, envioporta8, sizeof(envioporta8), 0, (struct sockaddr *)&server_address7, sizeof(server_address7)); // 4->7
                    if (x > 0)
                    {
                        len = sizeof(server_address8);
                        x = recvfrom(sock5, &recebeporta5, 1, 0, (struct sockaddr *)&server_address7, &len); // Controle 5<-7
                        if (x > 0 && recebeporta5 == '4')
                        {
                            printf("Você foi desconectado do chat.\n"); // Controle
                            conectado = 0;
                        }
                        else if (x > 0 && recebeporta5 == '1')
                        {
                            printf("Voce nao estava na lista\n"); // Controle
                            conectado = 0;
                        }
                        else
                            printf("Erro\n");
                    }
                    else
                        printf("Erro, ao enviar\n");
                }
                else if (strcmp(cod, "/msg") == 0){// MSG --------------------------------------------------------------------------------------
                    sendto(sock4, envioporta8, strlen(envioporta8) + 1, 0, (struct sockaddr *)&server_address8, sizeof(server_address8)); // 4->8
                    // len = sizeof(server_address8);
                    // x = recvfrom(sock5, &recebeporta5, 1, 0, (struct sockaddr *)&server_address8, &len); // Controle 5<-8
                    // if (x > 0)
                    // {
                    //     if (recebeporta5 == '2')
                    
                    //     else if (recebeporta5 == '1') 
                    //         printf("%s nao foi encoontrado no chat\n", nome); // Controle
                    // }
                    // else
                    //     printf("Erro\n");
                
                    // implementar essa proxima parte separado no processo do reciver
                    len = sizeof(server_address8);
                    if (recvfrom(sock4, recebeporta4, sizeof(recebeporta4) - 1, 0, (struct sockaddr *)&server_address8, &len) > 0) { // Controle 4<-8
                        recebeporta4[strlen(recebeporta4)] = '\0';
                        printf("%s\n", recebeporta4);
                    }
                    else printf("Erro\n");
                }
                else if (strcmp(cod, "/mpv") == 0){// MPV ---------------------------------------------------------------------------------------
                    sendto(sock4, envioporta8, strlen(envioporta8) + 1, 0, (struct sockaddr *)&server_address8, sizeof(server_address8)); // 4->8
                    len = sizeof(server_address8);
                    if (recvfrom(sock5, &recebeporta5, 1, 0, (struct sockaddr *)&server_address8, &len)> 0){// Controle 5<-8
                        if (recebeporta5 == '2') printf("Enviado.\n"); // Controle
                        else if (recebeporta5 == '3') printf("%s nao foi encoontrado no chat\n", nome); // Controle
                    }
                    else printf("Erro\n");

                    // implementar essa proxima parte separado no processo do reciver
                    len = sizeof(server_address8);
                    if (recvfrom(sock4, recebeporta4, sizeof(recebeporta4) - 1, 0, (struct sockaddr *)&server_address8, &len) > 0){
                        recebeporta4[strlen(recebeporta4)] = '\0';
                        printf("%s\n", recebeporta4);
                    }
                }
                else if (strcmp(cod, "/hlp") == 0){ // HLP/ajuda ---------------------------------------------------------------------------------------
                    printf("--------------------------------------------------------\n");
                    printf("/lin <nikname[9]> : Loga e salva seu nome. Se já estiver logado, informará que você já está logado e não poderá trocar o nome.\n");
                    printf("/out : Sai do chat.\n");
                    printf("/sai : Sai e encerra o programa.\n");
                    printf("/msg <mensagem[128]> : Envia uma mensagem com tamanho de até 127 caracteres.\n");
                    printf("/mpv <nikname[9]> <mensagem[127]> : Envia uma mensagem privada com tamanho de até 127 caracteres para o usuário especificado. Se o usuário não existir, você será informado.\n");
                    printf("/arq <caminho do arquivo> Envia um arquivo de até 1500 bytes.\n");
                    printf("/hlp : Ajuda/Printa opcoes\n");
                    printf("--------------------------------------------------------\n");
                }
            }
        
            /*--------------------------------------------------------------------------------------------------------------------------------------------------*/
            // Envio de Arquivo ----------------------------------------------------------------------------------------------------------------------------------
            else if (segmnta_arq(txt, cod, caminho)){ // arq --------------------------------------------------------------------------------------------------------------------
                if (le_arq(caminho, tam, ext, arquivo, nome_arq)){
                    // printf("%s %s %s %s", caminho, tam, ext, nome_arq); //debug
                    sprintf(envioporta8, "%s %s %s %s", cod, nome_arq, ext, tam);
                    sendto(sock4, envioporta8, strlen(envioporta8) + 1, 0, (struct sockaddr *)&server_address8, sizeof(server_address8)); // Enviar para porta 50008 4->8

                    // Receber resposta do servidor na porta 5 50005 = 50005
                    len = sizeof(server_address8);
                    x = recvfrom(sock5, &recebeporta5, 1, 0, (struct sockaddr *)&server_address8, &len); // 5<-8
                    if ((x > 0) && recebeporta5 == '2'){//Permissao para mandar arquivo
                        sendto(sock4, arquivo, sizeof(arquivo), 0, (struct sockaddr *)&server_address8, sizeof(server_address8)); // Envia arq 4->8
                        
                        // Recebimento de Arquivo-----------------------------------------------------------------------------------------------------------
                        /*Num chat isso seria uma thread/processo, porem aqui sera um bate e volta*/

                        // Receber resposta do servidor na porta 4 50004
                        len = sizeof(server_address8);
                        recvfrom(sock5, &recebeporta5, 1, 0, (struct sockaddr *)&server_address8, &len); // 5->8

                        if (recebeporta5 == '6'){ // 2 recebimento de arq
                            len = sizeof(server_address8);
                            bytes = recvfrom(sock4, recebeporta4, sizeof(recebeporta4) - 1, 0, (struct sockaddr *)&server_address8, &len); // 4<-8
                            if (bytes == -1)perror("Erro ao receber dados");
                            else{
                                recebeporta4[bytes] = '\0'; // Adicionar o null-terminator manualmente
                                printf("Recebido do servidor na porta 4: %s\n", recebeporta4);
                                // Reescreve variaveis---------------------------------------
                                if (segmenta_arq_recebimento(recebeporta4, nome_arq, ext, tam)) {
                                    strcat(nome_arq, "(2)");
                                    len = sizeof(server_address8);
                                    x = recvfrom(sock4, arquivo_in, sizeof(arquivo_in) - 1, 0, (struct sockaddr *)&client_address5, &len); // 4<-8
                                    arquivo_in[x] = '\0';
                                    printf("%s\n", arquivo_in);
                                    if (salva_arquivo(arquivo_in, nome_arq, caminho_in, ext)) printf("Arquivo salvo com sucesso!\n");
                                    else printf("Falha ao salvar o arquivo.\n");
                                }
                                else printf("Erro\n");
                            }
                        }
                    }
                    else if ((x > 0) && recebeporta5 == '0'){
                        printf("Usuario nao esta no chat\n");
                        conectado = 0;
                    }
                    else printf("Erro\n");
                }
                
                else printf("Erro em ler o arquivo\n");
        }

        /*--------------------------------------------------------------------------------------------------------------------------------*/

         
        else printf("Falha ao processar o texto.\n");
        }
    }
        /*-------------------------------------------------------------------------------------------------------------------------------*/
    

    close(sock4);
    close(sock5);

    return 0;
}
