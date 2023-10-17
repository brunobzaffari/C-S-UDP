#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>


#define INATIVO "INATIVO"
#define ATIVO "ATIVO"
#define MAX_VALUE 3

/*Segmenta de acordo com a func----------------------------------------------------------------------------------------------------------------*/


bool segmenta_recebeporta(char *recebeporta, char *cod, char *nome, char *mensagem, int *tam, char *ext, char *nome_arq) {
    if (strncmp(recebeporta, "/lin", 4) == 0) {// /LIN ---------------------------------------------------------------------------------------
		if(sscanf(recebeporta, "%4s %9s", cod, nome)==2) return true;
    }  
    if (strncmp(recebeporta, "/out", 4) == 0) {// OUT ---------------------------------------------------------------------------------
        strcpy(cod, "/out"); return true;
    }  
    if (strncmp(recebeporta, "/sai", 4) == 0) {// /SAI ---------------------------------------------------------------------------------------
        strcpy(cod, "/sai"); return true;
    }
    if (strncmp(recebeporta, "/msg", 4) == 0) {// /MSG ---------------------------------------------------------------------------------------
		if(sscanf(recebeporta, "%4s %111s", cod, mensagem)==2) return true;
    }  
    if (strncmp(recebeporta, "/mpv", 4) == 0) {// /MPV ---------------------------------------------------------------------------------------
        if(sscanf(recebeporta, "%4s %9s %111s", cod, nome, mensagem)==3) return true;
    }  
    if (strncmp(recebeporta, "/arq", 4) == 0) {// /ARQ ---------------------------------------------------------------------------------------
        if(sscanf(recebeporta, "%4s %19s %9s %d", cod, nome_arq, ext, tam) == 4) return true;
    }
    return false;
}

/*Dessegmenta de acordo com a func----------------------------------------------------------------------------------------------------------------*/
bool dessegmenta(char *envioporta4, const char *nome_arq, const char *ext, int tam) {//Para /arq tb mas para envio
    if (sprintf(envioporta4, "%s %s %d", nome_arq, ext, tam) > 0) 
        return true;
    
    return false;}
/*--------------------------------------------------------------------------------------------------------------------------------------------*/

// Estrutura para armazenar informações de cliente em uma lista ligada

struct ClientNode {
	//const char* state; // Campo para o estado do cliente, pode ser ATIVO ou INATIVO
	char ip_address[16];
    char name[10];
    struct ClientNode* next;
};

struct ClientNode* head = NULL;  // Cabeça da lista ligada
/*--------------------------------------------------------------------------------------------------------------------------------------------*/

int get_list_size() {
    int count = 0;
    struct ClientNode* temp = head;

    while (temp != NULL) {
        count++;
        temp = temp->next;
    }

    return count;
}

bool find_client(char *ip_address, char *client_name) {
	
	
    // if (client_name == NULL) {
    //     return false;
    // }

    struct ClientNode* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->ip_address, ip_address) == 0) {
            size_t name_length = strlen(temp->name);
            strncpy(client_name, temp->name, name_length);
            client_name[name_length] = '\0';
            return true;
        }
        temp = temp->next;
    }

    client_name[0] = '\0';
    return false;
}

void clear_client_list() {// garante a lista n ter elementos
    struct ClientNode* current = head;
    struct ClientNode* next_node;

    while (current != NULL) {
        next_node = current->next;  // Salva o próximo nó
        free(current);  // Libera a memória do nó atual
        current = next_node;  // Move para o próximo nó
    }

    head = NULL;  // Define a cabeça como NULL, tornando a lista vazia
}

bool is_client_in_list(const char *ip_address) {
    // Inicia o ponteiro temporário na cabeça da lista
    struct ClientNode* temp = head;
    
    // Loop até o final da lista
    while (temp != NULL) {
        // Compara o endereço IP
        if (strcmp(temp->ip_address, ip_address) == 0) {
            return true;  // Retorna verdadeiro se o IP for encontrado
        }
        temp = temp->next;  // Move para o próximo nó
    }
    
    return false;  // Retorna falso se o IP não for encontrado
}

void remove_client(char *ip_address) {
    struct ClientNode *temp = head, *prev;

    if (temp != NULL && strcmp(temp->ip_address, ip_address) == 0) {
        head = temp->next;
        free(temp);
        return;
    }

    while (temp != NULL && strcmp(temp->ip_address, ip_address) != 0) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    prev->next = temp->next;
    free(temp);
}

bool insert_client(char *ip_address, const char *name) {
  
    struct ClientNode* new_node = (struct ClientNode*)malloc(sizeof(struct ClientNode));
    if (new_node == NULL) {
        return false;
    }

    // Copiar ip_address
    strncpy(new_node->ip_address, ip_address, sizeof(new_node->ip_address) - 1);
    new_node->ip_address[sizeof(new_node->ip_address) - 1] = '\0';

    // Copiar name
    strncpy(new_node->name, name, sizeof(new_node->name) - 1);
    new_node->name[sizeof(new_node->name) - 1] = '\0';


    new_node->next = head;
    head = new_node;

    return true;
}

bool find_client_by_name(const char *name, char *client_ip) {
    if (name == NULL || client_ip == NULL) {
        return false;
    }

    struct ClientNode* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            strcpy(client_ip, temp->ip_address);
            return true;
        }
        temp = temp->next;
    }

    return false;
}


void print_client_list() {
    struct ClientNode* temp = head;
    int count = 1;  // Para numerar os clientes

    if (head == NULL) {
        printf("Lista de clientes está vazia.\n");
        return;
    }

    printf("Lista de Clientes:\n");
    printf("---------------------------------------------------\n");
    while (temp != NULL) {
        printf("Cliente %d:\n", count);
        //printf("Estado: %s\n", temp->state);
        printf("Endereço IP: %s\n", temp->ip_address);
        printf("Nome: %s\n", temp->name);
        printf("---------------------------------------------------\n");

        count++;
        temp = temp->next;
    }
}
// Função para enviar um broadcast para todos os clientes na lista
void send_to_all_clients(int sock, const void *envioporta4, int porta, ssize_t len ) {
    int flags = 0;

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(porta);  // Substitua com a porta correta

    struct ClientNode* temp = head;

    do{
        inet_pton(AF_INET, temp->ip_address, &dest_addr.sin_addr);// Configurar o endereço IP
       
        ssize_t bytes_sent = sendto(sock, envioporta4, len, flags, (struct sockaddr *)&dest_addr, sizeof(dest_addr)); // Enviar dados
        
        if (bytes_sent == -1) perror("Erro ao enviar dados para um cliente");// Checar por erros

        temp = temp->next;
    }while (temp != NULL) ;
}

int main() {
    struct sockaddr_in server_address7, server_address8;
    struct sockaddr_in client_address, dest_addr;
    ssize_t bytes, bytes2;
    char recebeporta7[128];
    char recebeporta8[128];
    char envioporta4[1501];//mensagens 
    char mensagem[112];
    char client_ip2[16]; // Buffer para armazenar o endereço IP em formato string
	char client_ip[16]; // Buffer para armazenar o endereço IP em formato string
    fd_set readfds;
    socklen_t len;
     
     
    char nome[10];
    char cod[5];
    //bool esta_na_lista;
    //int tamanho_lista;

    /*Para recebimento de arquivo------------------------------------------------------------------------------------------------------------*/
    char arquivo[1501];
    char nome_arq[20];
    char ext[10];
    int tam;
    

    clear_client_list();

    /*---------------------------------------------------------------------------------------------------------------------------------------*/
    // Criar o primeiro socket (porta 7) 50007
    int sock7 = socket(AF_INET, SOCK_DGRAM, 0);
    server_address7.sin_family = AF_INET;
    server_address7.sin_port = htons(50007);
    server_address7.sin_addr.s_addr = INADDR_ANY;
    bind(sock7, (struct sockaddr*)&server_address7, sizeof(server_address7));

    // Criar o segundo socket (porta 8) 50008
    int sock8 = socket(AF_INET, SOCK_DGRAM, 0);
    server_address8.sin_family = AF_INET;
    server_address8.sin_port = htons(50008);
    server_address8.sin_addr.s_addr = INADDR_ANY;
    bind(sock8, (struct sockaddr*)&server_address8, sizeof(server_address8));
    
    int max_sock = (sock7 > sock8 ? sock7 : sock8) + 1;

    while (1) {

        FD_ZERO(&readfds);
        FD_SET(sock7, &readfds);
        FD_SET(sock8, &readfds);
        //Zera variaveis para garantir---------------------------------------------------------------------------------------------------
        memset(&client_address, 0, sizeof(client_address));
        memset(recebeporta7, 0, sizeof(recebeporta7));
        memset(recebeporta8, 0, sizeof(recebeporta8));
        memset(envioporta4, 0, sizeof(envioporta4));
        memset(client_ip2, 0, sizeof(client_ip2));
        memset(client_ip, 0, sizeof(client_ip));
        memset(mensagem, 0, sizeof(mensagem));
        memset(arquivo, 0, sizeof(arquivo));
        memset(nome, 0, sizeof(nome));
        memset(cod, 0, sizeof(cod));
        bytes2 = -2;
        bytes = -2;
       
        // Usar select para esperar por qualquer atividade nos sockets
        if (select(max_sock, &readfds, NULL, NULL, NULL) < 0) {
            perror("Erro no select");
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(sock7, &readfds)) {
            // Recebe dados do primeiro socket e processa
            len = sizeof(client_address);
            client_address.sin_port = htons(50004);
            // Escutar na porta 7 50007
            bytes = recvfrom(sock7, recebeporta7, sizeof(recebeporta7) -1, 0, (struct sockaddr*)&client_address, &len);
            if (bytes == -1) perror("erro");
            //printf("Recebido na porta 7: %s\n", recebeporta7);
            //Recebe texto-------------------------------------------------------------------------------------------------------------------
            else {
                recebeporta7[bytes]='\0';
                inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, sizeof(client_ip));
                printf("Recebido na porta 50007: %s\n", recebeporta7);
                printf("Mensagem recebida de %s, com tamanho %d\n", client_ip, len);

                /*------------------------------------------------------------------------------------------------------------------------------*/
                segmenta_recebeporta(recebeporta7, cod, nome, mensagem, &tam, ext, nome_arq);// somente na porta 7 ocorre a entrada e saida de clientes
                if(strcmp(cod, "/lin") == 0){//LIN-------------------------------------------------------------------------------------------------------------------
                    if(!(is_client_in_list(client_ip))){//N esta na lista
                        if(get_list_size()<=MAX_VALUE){//Se nao chegou no tamanho maximo. (Definido como numero maximo da lista 3)
                            insert_client(client_ip, nome);
                            printf("--------------->Inserção de cliente!\n");
                            print_client_list();//ativar?

                            client_address.sin_port = htons(50005); // Responder ao cliente na porta 5 50005
                            sendto(sock7, "2", 1, 0, (struct sockaddr*)&client_address, len);//Codigo de controle para avisar q foi inserido 5<-7
                        }
                        else{
                            client_address.sin_port = htons(50005); 
                            sendto(sock7, "3", 1, 0, (struct sockaddr*)&client_address, len);//Codigo de controle para avisar q a lista esta cheia  5<-7
                        }
                    }
                    else{//Ja esta na lista
                        find_client(client_ip, nome);
                        client_address.sin_port = htons(50005); // Responder ao cliente na porta 5 50005
                        len = sizeof(client_address);
                        sendto(sock7, "4", 1, 0, (struct sockaddr*)&client_address, len);// avisa q ja esta na lista  5<-7
                        
                        len = sizeof(client_address);
                        sprintf(envioporta4, "Voce ja esta conectado com o nome: %s", nome);
                        client_address.sin_port = htons(50004); 
                        sendto(sock7, envioporta4, strlen(envioporta4) + 1, 0, (struct sockaddr*)&client_address, len); //retorna o nome  4<-7
                    }
                }
                else if(is_client_in_list(client_ip)){
                    if(strcmp(cod, "/sai") == 0){//SAI --------------------------------------------------------------------------------------------------------
                        //Funcao para deixar cliente suspenso;
                        printf("--------------->Suspensao de cliente!\n");
                        print_client_list();
                        len = sizeof(client_address);
                        client_address.sin_port = htons(50005); // Responder ao cliente na porta 5 50005
                        sendto(sock7, "2", 1, 0, (struct sockaddr*)&client_address, len);//Codigo de controle para avisar q sabe q o cliente saiu  5<-7
                
                    }
                    else if((strcmp(cod, "/out") == 0) &&  (is_client_in_list(client_ip))){//OUT  --------------------------------------------------------------------------------------------------------
                        remove_client(client_ip);
                        printf("--------------->Remoção de cliente!\n");
                        print_client_list();
                        client_address.sin_port = htons(50005); // Responder ao cliente na porta 5 50005
                        len = sizeof(client_address);
                        sendto(sock7, "4", 1, 0, (struct sockaddr*)&client_address, len);//Codigo de controle para avisar q sabe q o cliente deslogou 5<-7
                    }
                    else{
                        client_address.sin_port = htons(50005); //coigo de controle ouve algum erro
                        len = sizeof(client_address);
                        sendto(sock7, "1", 1, 0, (struct sockaddr*)&client_address, len); //5<-7 1=Cliente nao esta na lista
                    }
                }
                else{
                    client_address.sin_port = htons(50005); //coigo de controle ouve algum erro
                    len = sizeof(client_address);
                    sendto(sock7, "0", 1, 0, (struct sockaddr*)&client_address, len); //5<-7 0 = codigo de erro -> Mensagem invalida
                }
                
            }
        }
        /*--------------------------------------------------------------------------------------------------------------------------------*/
        if (FD_ISSET(sock8, &readfds)) {
            // Recebe dados do segundo socket e processa
            len = sizeof(client_address);
            client_address.sin_port = htons(50004);
            // Escutar na porta 8 50008
           
            bytes2= recvfrom(sock8, recebeporta8, sizeof(recebeporta8) -1, 0, (struct sockaddr*)&client_address, &len);
            
            if (bytes2 == -1) perror("erro");
            //Na porta 50008 ocorre troca de mensagens------------------------------------------------------------------------------------------------------------------------------*/
            else{
                recebeporta8[bytes2]='\0';
                inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, sizeof(client_ip));
                printf("Recebido na porta 8: %s\n", recebeporta8);
                printf("Mensagem recebida de %s, com tamanho %d\n", client_ip, len);
                /*------------------------------------------------------------------------------------------------------------------------------*/
                if(!(is_client_in_list(client_ip))){//Nao esta na lista
                    client_address.sin_port = htons(50005); //coigo de controle ouve algum erro
                    len = sizeof(client_address);
                    sendto(sock7, "1", 1, 0, (struct sockaddr*)&client_address, len); //5<-7 1=Cliente nao esta na lista
                }
                else{
                    segmenta_recebeporta(recebeporta8, cod, nome, mensagem, &tam, ext, nome_arq);
                    //printf("%s %ld\n", cod, strlen(mensagem)); //debug
                    if(strcmp(cod, "/lin") == 0){//LIN-------------------------------------------------------------------------------------------------------------------
                        //Ja esta na lista
                        find_client(client_ip, nome);
                        client_address.sin_port = htons(50005); // Responder ao cliente na porta 5 50005
                        len = sizeof(client_address);
                        sendto(sock8, "4", 1, 0, (struct sockaddr*)&client_address, len);// avisa q ja esta na lista 5<-8
                        sprintf(envioporta4, "Voce ja esta conectado com o nome: %s", nome);
                
                        client_address.sin_port = htons(50004); 
                        len = sizeof(client_address);
                        sendto(sock8, envioporta4, strlen(envioporta4) + 1, 0, (struct sockaddr*)&client_address, len); //retorna o nome 4<-8
                    }
                    if(strcmp(cod, "/msg") == 0){//MSG  ------------------------------------------------------------------------------------------------------
                        memset(envioporta4, 0, sizeof(envioporta4));
                        find_client(client_ip, nome);
                        //printf("<%s>: %s\n", nome, mensagem); //debug
                        // client_address.sin_port = htons(50005); // Responder ao cliente na porta 5 50005
                        // len = sizeof(client_address);
                        // sendto(sock8, "2", 1, 0, (struct sockaddr*)&client_address, len);// avisa q ja esta na lista 5<-8
                        sprintf(envioporta4, "<%s>: %s", nome, mensagem);
                        printf("%s", envioporta4);
                        send_to_all_clients(sock8, envioporta4, 50004, strlen(envioporta4));
                        // client_address.sin_port = htons(50004); 
                        // len = sizeof(client_address);
                        // sendto(sock8, envioporta4, strlen(envioporta4) +1, 0, (struct sockaddr*)&client_address, len);
                    }
                    if(strcmp(cod, "/mpv") == 0){//MPV  -------------------------------------------------------------------------------------------------------
                        memset(&dest_addr, 0, sizeof(dest_addr));
                        
                        dest_addr.sin_family = AF_INET;
                        dest_addr.sin_port = htons(50004);  // Substitua com a porta correta

                        inet_pton(AF_INET, client_ip2, &dest_addr.sin_addr);// Configurar o endereço IP

                        if(find_client_by_name(nome, client_ip2)){
                            inet_pton(AF_INET, client_ip2, &dest_addr.sin_addr);// Configurar o endereço IP
                            //client_address.sin_port = htons(50004); // Responder ao cliente na porta 4 50004
                            len = sizeof(dest_addr);
                            memset(nome, 0, sizeof(nome));
                            find_client(client_ip, nome);
                            sprintf(envioporta4, "MPV <%s>: %s", nome, mensagem);

                            sendto(sock8, envioporta4, strlen(envioporta4) + 1, 0, (struct sockaddr*)&dest_addr, len); // envia a mensagem privada 4<-8

                            client_address.sin_port = htons(50005); // Responder ao cliente na porta 5 50005
                            len = sizeof(client_address);
                            sendto(sock8, "2", 1, 0, (struct sockaddr*)&client_address, len);//Codigo de controle 2 = avisar q enviou 
                        }
                        else{
                            client_address.sin_port = htons(50005); 
                            len = sizeof(client_address);
                            sendto(sock8, "3", 1, 0, (struct sockaddr*)&client_address, len);//Codigo de controle 3 = avisar q nao enviou
                        }
                    }
                    if((strcmp(cod, "/arq")== 0)){//ARQ  -------------------------------------------------------------------------------------------------------------
                        client_address.sin_port = htons(50005); // Responder ao cliente na porta 5 50005
            
                        sendto(sock8, "2", 1, 0, (struct sockaddr*)&client_address, len);//coigo de controle 2 5<-8 para o cliente comecar a enviar
                        len = sizeof(client_address);
                        bytes = recvfrom(sock8, arquivo, sizeof(arquivo) -1, 0, (struct sockaddr*)&client_address, &len); //Recebe Arquivo 4->8
                        arquivo[bytes]='\0';

                        printf("%s\n", arquivo);

                        //Envia Arquivo--------------------------------------------------------------------------------------------------------------------
                        send_to_all_clients(sock8, "6", 50005, 1);// Avisa que vai transmitir um arquivo 5<-8

                        dessegmenta(envioporta4, nome_arq, ext, tam); //Formata para enviar

                        //broadcast
                        send_to_all_clients(sock8, envioporta4, 50004, (strlen(envioporta4) + 1));// Responder aos clientes na porta 4 50004 4<-8
                        send_to_all_clients(sock8, arquivo, 50004, (strlen(arquivo) + 1));// Responder aos clientes na porta 4 50004 4<-8
                        //sendto(sock7, envioporta4, strlen(envioporta4) + 1, 0, (struct sockaddr*)&client_address, len); // Envia: nome_arq, ext e tam do arquivo
                        //sendto(sock7, arquivo, strlen(arquivo) + 1, 0, (struct sockaddr*)&client_address, len); // Envia arquivo
                        
                    }
                    
                }
            }
        }
    }
    /*--------------------------------------------------------------------------------------------------------------------------------*/

    close(sock7);
    close(sock8);

    return 0;

}
