//
// Created by mfarr on 21/02/2022.
//

//Server file
#include "netinet/in.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "constants.h"
#include "structs.h"
#include "netdb.h"
#include "unistd.h"
#include <pthread.h>
#include <stdbool.h>

#define LONGDADES 100

//Global variable
struct ClientConfig clientConfiguration;
char* client_state = NULL;
struct Sockets allSockets;
struct Server server;
pthread_t idThread = (pthread_t) NULL;
struct Client client;

void readClientConfig(FILE *file,int debug);
char* splitLast(char split[]);
void setupData(int argc,const char* argv[]);
void setup_udp_socket(char* udpPort);
struct UDP mountPdu(unsigned char type, char* IdTransmissor, char* IdCommunication, char* Data);
void connection();
unsigned char get_state_into_unChar(char *state);
void change_client_state(char *newState);
void send_package_udp(struct UDP request,char *port);
struct UDP recive_package_UDP(int max_timeout);
void save_server_data(struct UDP recivedPackage);
void* keep_alive();
bool checkAlivePackage(struct UDP recivedAlivePackage);
bool checkServerInfo(struct UDP recivedServerPackage);


int main(int argc, const char* argv[]){
    struct UDP test;
    setupData(argc,argv);

    //Start with register
    change_client_state("NOT_REGISTERED");
    setup_udp_socket(clientConfiguration.server_UDP_port);
    connection();
    //Send Alive
    pthread_create(&idThread,NULL,keep_alive(),NULL);


}
void setupData(int argc,const char* argv[]){
    int debug = 0;
    FILE *config_file = NULL;
    for(int index = 1;index<argc;index++){

        if(strcmp(argv[index],"-d")==0){
            debug = 1;
        }else if(strcmp(argv[index],"-c")==0){
            config_file = fopen(argv[index+1],"r");
        }
    }
    if(config_file==NULL){
        config_file = fopen("client.cfg","r");
        if(config_file==NULL){
            printf("ERROR can not read file");
        }
    }
    readClientConfig(config_file,debug);
}


void readClientConfig(FILE *file,int debug){
    char line[50];
    char* split;
    char* token;

    while(fgets(line,50,file)) {
        char *c = strchr(line, '\n');
        if (c) {
            *c = '\0';
        }
        split = strtok(line, " ");

        if (strcmp(split, "Id") == 0) {
            strcpy(clientConfiguration.clientID, splitLast(split));
        } else if (strcmp(split, "Params") == 0) {
            char *param;
            char *lastParams = splitLast(line);
            param = strtok(lastParams, ";");
            int i = 0;
            while (param != NULL) {
                strcpy(clientConfiguration.params[i], param);
                param = strtok(NULL, ";");
                i++;
            }

        } else if (strcmp(split, "Local-TCP") == 0) {
            strcpy(clientConfiguration.local_TCP_port, splitLast(split));
        } else if (strcmp(split, "Server") == 0) {
            token = splitLast(split);
            clientConfiguration.server_adress = malloc(strlen(token));
            strcpy(clientConfiguration.server_adress, token);
        } else if (strcmp(split, "Server-UDP") == 0) {
            strcpy(clientConfiguration.server_UDP_port, splitLast(split));
        }
    }
    clientConfiguration.debug = debug;
    if(clientConfiguration.debug ==1){
        printf("Configuration added correctly\n");
    }
}

char *splitLast(char split[]) {
    for (int i = 0; i <= 1; i++) {
        split = strtok(NULL, " ");
    }
    return split;
}

unsigned char get_state_into_unChar(char *state){
    unsigned char state_type;
    if(strcmp(state,"REG_REQ")==0){
        state_type = (unsigned char) 0xa0;
    }else if(strcmp(state,"REG_ACK")==0){
        state_type = (unsigned char) 0xa1;
    }
}

char* get_state_into_str(unsigned char state){
    char *strState;
    if(state == (unsigned char) 0xa0){
        strState = "REG_REQ";
    }else if(state ==(unsigned char) 0xa1){
        strState="REG_ACK";
    }else if(state ==(unsigned char) 0xa2){
        strState="REG_NACK";
    }else if(state ==(unsigned char) 0xa3){
        strState="REG_REJ";
    }else if(state == (unsigned char)0xa4){
        strState="REG_INFO";
    }else if(state == (unsigned char)0xa5){
        strState="INFO_ACK";
    }else if(state == (unsigned char)0xa6){
        strState="INFO_NACK";
    }else if(state == (unsigned char)0xa7){
        strState="INFO_REJ";
    }else if(state == (unsigned char)0xf0){
        strState="DISCONNECTED";
    }else if(state == (unsigned char)0xf1){
        strState="NOT_REGISTERED";
    }else if(state == (unsigned char)0xf2){
        strState="WAIT_ACK_REG";
    }else if(state == (unsigned char)0xf3){
        strState="WAIT_INFO";
    }else if(state == (unsigned char)0xf4){
        strState="WAIT_ACK_INFO";
    }else if(state == (unsigned char)0xf5){
        strState="REGISTERED";
    }else if(state ==(unsigned char) 0xf6) {
        strState = "SEND_ALIVE";
    }else if(state ==(unsigned char) 0xb0) {
        strState = "ALIVE";
    }else if(state == (unsigned char) 0xb1) {
        strState = "ALIVE_NACK";
    }else if(state == (unsigned char) 0xb2){
        strState = "ALIVE_REJ";
    }else{
        strState="ERROR";
    }
    return strState;
}

void setup_udp_socket(char* udpPort){
    //create sockets

    struct hostent *ent;

    ent = gethostbyname(clientConfiguration.server_adress);
    if(!ent){
        printf("ERROR, can not take host name\n");
        exit(-1);
    }

    allSockets.udp_socket = socket(AF_INET, SOCK_DGRAM,0);
    if(allSockets.udp_socket < 0){
        printf("ERROR creating socket \n");
        exit(-1);
    }

    memset(&allSockets.client_adress,0,sizeof(struct sockaddr_in));
    allSockets.client_adress.sin_family = AF_INET;
    allSockets.client_adress.sin_addr.s_addr = htonl(INADDR_ANY);
    allSockets.client_adress.sin_port = htons(0);

    if(bind(allSockets.udp_socket,(struct sockaddr *)&allSockets.client_adress,sizeof(struct sockaddr_in))<0){
        printf("ERROR in BINDING\n");
        exit(-1);
    }

    memset(&allSockets.udp_addr_server,0,sizeof(struct sockaddr_in));
    allSockets.udp_addr_server.sin_family=AF_INET;
    allSockets.udp_addr_server.sin_addr.s_addr=(((struct in_addr *) ent->h_addr_list[0])->s_addr);
    allSockets.udp_addr_server.sin_port = htons(atoi(udpPort));
}

struct UDP mountPdu(unsigned char type, char* IdTransmissor, char* IdCommunication, char* Data){
    struct UDP regReq;
    regReq.type = (unsigned char) type;
    strcpy(regReq.idTransmissor,IdTransmissor); // Not passing correctly
    strcpy(regReq.idCommunication,IdCommunication);
    strcpy(regReq.data,Data);
    return regReq;
}

void connection(){
    client.unsucssesful_singUps = 0;
    fd_set rfds;
    while(client.unsucssesful_singUps < o){
        int max_timeout = t;
        if(clientConfiguration.debug == 1){
            //Escribir nuevo intento + numero de trys
        }
        for (int reg_sent = 0; reg_sent < n; reg_sent++){
            struct UDP registerReq;
            registerReq = mountPdu(REG_REQ,clientConfiguration.clientID,"0000000000","");
            send_package_udp(registerReq,clientConfiguration.server_UDP_port);
            change_client_state("WAIT_ACK_REG");
            struct UDP recivePackage = recive_package_UDP(max_timeout);

            if(strcmp(get_state_into_str(recivePackage.type),"REG_REJ")==0){
                change_client_state("NOT_REGISTERED");
                if(clientConfiguration.debug == 1){
                    printf("Package Rejected, trying sign up agains, try nº %i\n",client.unsucssesful_singUps);
                }
                client.unsucssesful_singUps++;
                break;
            }else if(strcmp(get_state_into_str(recivePackage.type),"REG_NACK")==0) {
                if (clientConfiguration.debug == 1) {
                    printf("Package Not accepted, trying another request, try nº %i", reg_sent);
                }
                change_client_state("NOT_REGISTERED");
            }else if(strcmp(get_state_into_str(recivePackage.type),"ERROR")==0){
                if(clientConfiguration.debug==1){
                    printf("ERROR time %i exceeded, trying again, try %i\n",max_timeout,reg_sent+1);
                }
                change_client_state("NOT_REGISTERED");
            }else{
                if(strcmp(client_state,"WAIT_ACK_REG")==0 && strcmp(get_state_into_str(recivePackage.type),"REG_ACK")==0){
                    change_client_state("WAIT_ACK_INFO");
                    save_server_data(recivePackage);
                    char data[61];
                    strcpy(data,clientConfiguration.local_TCP_port);
                    strcat(data,",");
                    for(int i = 0;i < 5;i++){
                        strcat(data,clientConfiguration.params[i]);
                        if(strcmp(&clientConfiguration.params[i+1][0],"\0")==0){

                            break;
                        }
                        strcat(data,";");
                    }
                    //Construir data pack
                    struct UDP info_packet = mountPdu(REG_INFO,clientConfiguration.clientID,server.ServerCommunication,data);
                    send_package_udp(info_packet,server.udp_port);
                    struct UDP recivedInfoPackage = recive_package_UDP(2*t);
                    if(strcmp(get_state_into_str(recivedInfoPackage.type),"ERROR")==0){
                        if(clientConfiguration.debug == 1){
                            printf("ERROR time %i for REG_INFO exceeded\n",2*t);
                        }
                        change_client_state("NOT_REGISTERED");
                        client.unsucssesful_singUps++;
                        break;
                    }else if(strcmp(get_state_into_str(recivedInfoPackage.type),"INFO_NACK")==0){
                        printf("ERROR,package info not accepted because:\n %s \n",recivedInfoPackage.data);
                        change_client_state("NOT_REGISTERED");
                    }else{
                        if(!checkServerInfo(recivedInfoPackage)){
                            printf("Error, incorrect server INFO\n");
                            exit(-1);
                        }
                        change_client_state("REGISTERED");
                        strcpy(server.tcp_port,recivedInfoPackage.data);
                        break;
                    }
                }
            }
            if(reg_sent > p-1){
                if(max_timeout < q*t){
                    max_timeout = max_timeout+t;
                }
            }
            if(reg_sent == n-1){
                printf("%i packages sended wrong trying again to sign up\n",n);
            }
        }
        if(strcmp(client_state,"REGISTERED")==0){
            break;
        }
        if(client.unsucssesful_singUps == o-1){
            printf("ERROR can not conect to the server\n");
            exit(-1);
        }
        client.unsucssesful_singUps++;
        change_client_state("NOT_REGISTERED");
        sleep(u);
    }
    close(allSockets.udp_socket);
}

void save_server_data(struct UDP recivedPackage){
    strcpy(server.ServerId,recivedPackage.idTransmissor);
    strcpy(server.ServerCommunication,recivedPackage.idCommunication);
    strcpy(server.udp_port,recivedPackage.data);
}

void change_client_state(char *newState){
    client_state = malloc(sizeof(&newState));
    strcpy(client_state,newState);
    printf("Client state changed to: %s\n",client_state);
}

void send_package_udp(struct UDP request,char* port){
    setup_udp_socket(port);
    int a = sendto(allSockets.udp_socket,&request,sizeof(request),0,
                   (struct sockaddr*)&allSockets.udp_addr_server,sizeof(allSockets.udp_addr_server));
    //char message[61];
    if(a<0){
        printf("ERROR SENDING PACKAGE TO UDP\n");
        exit(-1);
    }
}

struct UDP recive_package_UDP(int max_timeout){
    struct UDP *recivedPackage = malloc(sizeof (struct UDP));
    struct timeval timeout;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(allSockets.udp_socket,&rfds);
    timeout.tv_sec = t;
    timeout.tv_usec = 0;
    select(allSockets.udp_socket+1,&rfds,NULL,NULL,&timeout);
    if(FD_ISSET(allSockets.udp_socket,&rfds)){
        char *buf = malloc(sizeof(struct UDP));
        int a = recvfrom(allSockets.udp_socket,buf,sizeof(struct UDP),0,
                         (struct sockaddr *)0,(socklen_t *)0);
        if(a<0) {
            if (clientConfiguration.debug == 1) {
                printf("ERROR Cannot recive a message\n");
            }
            exit(-1);
        }
        recivedPackage = (struct UDP *) buf;
    }else{
        recivedPackage->type = ERROR;
        return *recivedPackage;
    }

    return *recivedPackage;
}

void * keep_alive(){
    struct timeval timeout;
    int aliveNotConsec = 0;
    while (1) {
        if(aliveNotConsec > s){
            printf("ERROR didn't recived 3 alive consecutly, retrying register");
            change_client_state("NOT_REGISTERED");
            client.unsucssesful_singUps++;
            connection();
            pthread_cancel(idThread);
        }
        int time = r;
        struct UDP alivePacket = mountPdu(ALIVE,clientConfiguration.clientID,server.ServerCommunication,"");
        send_package_udp(alivePacket,clientConfiguration.server_UDP_port);
        struct UDP recivedAlivePacket = recive_package_UDP(time*v);
        if(!checkAlivePackage(recivedAlivePacket)){
            printf("ERROR suplantation identity\n");
            change_client_state("NOT_REGISTERED");
            client.unsucssesful_singUps++;
            connection();
            pthread_exit(&idThread);
        }
        if(strcmp(get_state_into_str(recivedAlivePacket.type),"ALIVE_REJ")==0){
            printf("Packet Rejected, trying sing up again\n");
            change_client_state("NOT_REGISTERED");
            client.unsucssesful_singUps++;
            connection();
            pthread_exit(&idThread);
        }else if(strcmp(get_state_into_str(recivedAlivePacket.type),"ERROR")==0){
            if(strcmp(client_state,"REGISTERED")==0){
                printf("ERROR didn't recived first Alive\n");
                change_client_state("NOT_REGISTERED");
                client.unsucssesful_singUps++;
                connection();
                pthread_exit(&idThread);
            }
            if(clientConfiguration.debug == 1){
                printf("Not recived alive packet");
                aliveNotConsec++;
            }
        }else{
            if(strcmp(client_state,"REGISTERED")==0 || strcmp(client_state,"SEND_ALIVE")==0){
                if(strcmp(client_state,"REGISTERED")==0){
                    change_client_state("SEND_ALIVE");
                    //obrir socket tcp
                    aliveNotConsec = 0;
                }
            }
        }
        sleep(v);
    }
}

bool checkAlivePackage(struct UDP recivedAlivePackage){
    return (strcmp(recivedAlivePackage.idTransmissor,server.ServerId)==0 &&
            strcmp(recivedAlivePackage.idCommunication,server.ServerCommunication)==0 &&
            strcmp(recivedAlivePackage.data,clientConfiguration.clientID) == 0);
}

bool checkServerInfo(struct UDP recivedServerPackage){
    return (strcmp(recivedServerPackage.idTransmissor,server.ServerId)==0 &&
            strcmp(recivedServerPackage.idCommunication,server.ServerCommunication)==0);
}

void commandSystem(){
    while(strcmp(client_state,"SEND_ALIVE")==0){

    }
}
