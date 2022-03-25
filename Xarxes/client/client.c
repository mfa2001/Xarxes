//
// Created by mfarr on 21/02/2022.
//

//client file
#include "netinet/in.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "constants.h"
#include "structs.h"
#include <netdb.h>
#include "unistd.h"
#include <threads.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>

#define BUFFSIZE 250

//Global variable
struct ClientConfig clientConfiguration;
char* client_state = NULL;
struct Sockets allSockets;
struct Server server;
thrd_t threads[4];
struct Client client;
struct Elements arr_elem[5];

bool checkAlivePackage(struct UDP recivedAlivePackage);
bool checkServerInfo(struct UDP recivedServerPackage);
char* splitLast(char split[]);
unsigned char get_state_into_unChar(char *state);
struct UDP mountPdu(unsigned char type, char* IdTransmissor, char* IdCommunication, char* Data);
struct UDP recive_package_UDP(int max_timeout);
struct TCP mountTcp(unsigned char type,char *elemId);
struct TCP recive_package_TCP(int max_timeout);
struct TCP listening();
void *keep_alive();
void readClientConfig(FILE *file,int debug);
void setupData(int argc,const char* argv[]);
void setup_udp_socket(char* udpPort);
void connection();
void change_client_state(char *newState);
void send_package_udp(struct UDP request,char *port);
void save_server_data(struct UDP recivedPackage);
void commandSystem();
void setup_tcp_socket();
void send_package_tcp(struct TCP request);
void command_send(char *elemId);
void setup_socket_for_listening();
void threat_listening_commands();


int main(int argc, const char* argv[]){
    setupData(argc,argv);
    change_client_state("NOT_REGISTERED");
    setup_udp_socket(clientConfiguration.server_UDP_port);
    connection();
    while(1){
        if(strcmp(client_state,"SEND_ALIVE")){
            break;
        }
    }
    setup_socket_for_listening();
    thrd_create(&threads[1],(thrd_start_t)threat_listening_commands,NULL);
    commandSystem();
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
                strcpy(arr_elem[i].strElem, param);
                strcpy(arr_elem[i].valElem,"NONE");
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
    }else if(state == (unsigned char) 0xc0) {
        strState = "SEND_DATA";
    }else if(state == (unsigned char) 0xc0) {
        strState = "SEND_DATA";
    }else if(state == (unsigned char) 0xc1) {
        strState = "DATA_ACK";
    }else if(state == (unsigned char) 0xc2) {
        strState = "DATA_NACK";
    }else if(state == (unsigned char) 0xc3) {
        strState = "DATA_REJ";
    }else if(state == (unsigned char) 0xc4) {
        strState = "SET_DATA";
    }else if(state == (unsigned char) 0xc5){
        strState = "GET_DATA";
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
        exit(0);
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
    strcpy(regReq.idTransmissor,IdTransmissor);
    strcpy(regReq.idCommunication,IdCommunication);
    strcpy(regReq.data,Data);
    return regReq;
}

void connection(){
    client.unsucssesful_singUps = 0;
    while(client.unsucssesful_singUps < o){
        int max_timeout = t;
        if(clientConfiguration.debug == 1){
            //Escribir nuevo intento + numero de trys
        }
        for (int reg_sent = 0; reg_sent < n; reg_sent++){
            struct UDP registerReq;
            registerReq = mountPdu(REG_REQ,clientConfiguration.clientID,"0000000000","");
            send_package_udp(registerReq,clientConfiguration.server_UDP_port);
            if(strcmp(client_state,"WAIT_ACK_REG")!=0){
                change_client_state("WAIT_ACK_REG");
            }
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
                //change_client_state("NOT_REGISTERED");
            }else{
                if(strcmp(client_state,"WAIT_ACK_REG")==0 && strcmp(get_state_into_str(recivePackage.type),"REG_ACK")==0){
                    change_client_state("WAIT_ACK_INFO");
                    save_server_data(recivePackage);
                    char data[61];
                    strcpy(data,clientConfiguration.local_TCP_port);
                    strcat(data,",");
                    for(int i = 0;i < 5;i++){
                        strcat(data,arr_elem[i].strElem);
                        if(strcmp(&arr_elem[i].strElem[0],"\0")==0){

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
            thrd_create(&threads[0],(thrd_start_t) keep_alive,NULL);
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
    struct timeval tv;
    tv.tv_sec = max_timeout;
    tv.tv_usec = 0;
    struct UDP *recivedPackage = malloc(sizeof (struct UDP));
    //Its not adding correctly the timeout
    if(setsockopt(allSockets.udp_socket,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0){
        printf("ERROR");
    }
    char *buf = malloc(sizeof(struct UDP));
    int a = recvfrom(allSockets.udp_socket,buf,sizeof(struct UDP),0,
                     (struct sockaddr *)0,(socklen_t *)0);
    if(a<0) {
        recivedPackage->type = ERROR;
        return *recivedPackage;
    }else{
        recivedPackage = (struct UDP *) buf;
        return *recivedPackage;
    }

}

void * keep_alive(){
    int aliveNotConsec = 0;
    while (1) {
        if(aliveNotConsec > s-1){
            printf("ERROR didn't recived 3 alive consecutly, retrying register\n\n");
            change_client_state("NOT_REGISTERED");
            client.unsucssesful_singUps++;
            close(allSockets.tcp_socket);
            close(allSockets.tcp_listening_socket);
            connection();
            thrd_exit(EXIT_FAILURE);

        }
        int time = r;
        struct UDP alivePacket = mountPdu(ALIVE,clientConfiguration.clientID,server.ServerCommunication,"");
        send_package_udp(alivePacket,clientConfiguration.server_UDP_port);
        struct UDP recivedAlivePacket = recive_package_UDP(time*v);
        if(strcmp(get_state_into_str(recivedAlivePacket.type),"ERROR")==0) {
            if (strcmp(client_state, "REGISTERED") == 0) {
                printf("ERROR didn't recived first Alive\n");
                change_client_state("NOT_REGISTERED");
                client.unsucssesful_singUps++;
                connection();
                close(allSockets.tcp_socket);
                close(allSockets.tcp_listening_socket);
                thrd_exit(EXIT_FAILURE);
            }
            if (clientConfiguration.debug == 1) {
                printf("Not recived alive packet\n");
                aliveNotConsec++;
            }
        }else if(!checkAlivePackage(recivedAlivePacket)){
            printf("ERROR suplantation identity\n");
            change_client_state("NOT_REGISTERED");
            client.unsucssesful_singUps++;
            connection();
            close(allSockets.tcp_socket);
            close(allSockets.tcp_listening_socket);
            thrd_exit(EXIT_FAILURE);
        }else if(strcmp(get_state_into_str(recivedAlivePacket.type),"ALIVE_REJ")==0){
            printf("Packet Rejected, trying sing up again\n");
            change_client_state("NOT_REGISTERED");
            client.unsucssesful_singUps++;
            connection();
            close(allSockets.tcp_socket);
            close(allSockets.tcp_listening_socket);
            thrd_exit(EXIT_FAILURE);
        }else{
            if(strcmp(client_state,"REGISTERED")==0 || strcmp(client_state,"SEND_ALIVE")==0){
                if(strcmp(client_state,"REGISTERED")==0){
                    change_client_state("SEND_ALIVE");
                    //obrir socket tcp
                    //create thread for listening
                    aliveNotConsec = 0;
                }
                aliveNotConsec = 0;
            }
        }
        sleep(v);
    }
}

void threat_listening_commands() {
    struct TCP send_listenig;
    if(clientConfiguration.debug==1){
        printf("Client listening for commands\n");
    }
    while (1) {
        while(strcmp(client_state,"SEND_ALIVE")==0) {
            struct TCP recv_list = listening();
            if (strcmp(get_state_into_str(recv_list.type),"GET_DATA")==0) {
                int i = 0;
                while(i>=5 || strcmp(&arr_elem[i].strElem[0],"\0")!=0){
                    if(strcmp(recv_list.element,arr_elem[i].strElem)==0){
                        send_listenig = mountTcp(DATA_ACK,arr_elem[i].strElem);
                        write(allSockets.socket_for_new_accept,&send_listenig,sizeof(send_listenig));
                        if(clientConfiguration.debug==1){
                            printf("Sended element correctly\n");
                        }
                        break;
                    }else{
                        i++;
                    }
                }
            } else if(strcmp(get_state_into_str(recv_list.type),"SET_DATA")==0){
                int i = 0;
                int j = 0;
                while(i>=5 || strcmp(&arr_elem[i].strElem[0],"\0")!=0) {
                    if (strcmp(recv_list.element, arr_elem[i].strElem) == 0) {
                        while(strcmp(&arr_elem[i].strElem[j],"\0")!=0){
                            if(strcmp(&arr_elem[i].strElem[j],"I")==0){
                                strcpy(arr_elem[i].valElem,recv_list.valor);
                                send_listenig = mountTcp(DATA_ACK,arr_elem[i].strElem);
                                write(allSockets.socket_for_new_accept,&send_listenig,sizeof(send_listenig));
                                if(clientConfiguration.debug==1){
                                    printf("Setted new value correctly\n");
                                }
                                break;
                            }else{
                                j++;
                            }
                        }
                        if(strcmp(arr_elem[i].valElem,recv_list.valor)==0){
                            break;
                        }
                    }else{
                        i++;
                    }
                }
            }else{
                printf("ERROR package type not recognized");
            }
        }
    }
}

struct TCP listening(){

    struct TCP *recvListenignPacket = malloc(sizeof (struct TCP));

    listen(allSockets.tcp_listening_socket,5);
    socklen_t sizeaddr = sizeof(allSockets.tcp_listening_addr_server);
    allSockets.socket_for_new_accept = accept(allSockets.tcp_listening_socket,(struct sockaddr *)&allSockets.tcp_listening_addr_server,&sizeaddr);
    char *buffer = malloc(sizeof (struct TCP));
    if(allSockets.socket_for_new_accept > 0){

        if(read(allSockets.socket_for_new_accept,buffer,sizeof(struct TCP))<0){
            printf("ERROR reading into listening commands");
            recvListenignPacket->type = ERROR;
            return *recvListenignPacket;
        }else{
            recvListenignPacket = (struct TCP *)buffer;
            return *recvListenignPacket;
        }
    }else{
        printf("Error cannot create socket for reading");
        recvListenignPacket->type = ERROR;
        return *recvListenignPacket;
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
struct TCP mountTcp(unsigned char type,char *elemId){
    struct TCP send_data;
    int i = 0;

    send_data.type = (unsigned char) type;
    strcpy(send_data.id_transmisor,clientConfiguration.clientID);
    strcpy(send_data.id_comunicacio,server.ServerCommunication);
    strcpy(send_data.element,elemId);
    while(strcmp(elemId,arr_elem[i].strElem)!=0){
        i++;
    }
    strcpy(send_data.valor,arr_elem[i].valElem);
    time_t timenow;
    time(&timenow);
    strcpy(send_data.info,ctime(&timenow));
    return send_data;
}

void send_package_tcp(struct TCP request){
    setup_tcp_socket();
    if(write(allSockets.tcp_socket,&request,sizeof (request))==-1){
        printf("ERROR al escribir en socket");
    }
}

void commandSystem(){
    char buffIn[BUFFSIZE+2];
    char *split;
    int i;
    while(1){
        while(strcmp(client_state,"SEND_ALIVE")==0) {
            i = 0;
            fgets((char*)buffIn,BUFFSIZE,stdin);

            char *c = strchr(buffIn, '\n');
            if (c) {
                *c = '\0';
            }
            split = strtok(buffIn," ");
            if(strcmp(split,"stat")==0) {
                while(i>=5 || strcmp(&arr_elem[i].strElem[0],"\0")!=0){
                    printf("Name: %s  ",arr_elem[i].strElem);
                    printf("Value: %s\n",arr_elem[i].valElem);
                    i++;
                }
            }else if(strcmp(split,"set")==0){
                split = strtok(NULL," ");
                i = 0;
                if(split == NULL){
                    printf("ERROR, no arguments,use set <param_name> <param_value>\n");
                }else {
                    while (i>=5 || strcmp(&arr_elem[i].strElem[0], "\0") != 0) {
                        if (strcmp(arr_elem[i].strElem, split) == 0) {
                            split = strtok(NULL, " ");
                            if (split == NULL) {
                                printf("ERROR with arguments use a valid <param_name> <param_value>\n");
                                break;
                            }else{
                                strcpy(arr_elem[i].valElem, split);
                                printf("Changed value\n");
                                break;
                            }
                        } else {
                            i++;
                        }
                    }
                    if(i>=5){
                        printf("ERROR didn't recognize this element\n");
                    }
                }
            }else if(strcmp(split,"send")==0){
                split = strtok(NULL," ");
                if(split == NULL){
                    printf("ERROR, no arguments,use send <param_name>\n");
                }else{
                    while (i>=5 || strcmp(&arr_elem[i].strElem[0], "\0") != 0) {
                        if (strcmp(arr_elem[i].strElem, split) == 0) {
                            command_send(split);
                            break;
                        }else{
                            i++;
                        }
                    }
                    if(i>=5){
                        printf("ERROR didn't recognize this element\n");
                    }
                }
            }else if(strcmp(buffIn,"quit")==0){
                close(allSockets.tcp_socket);
                exit(-1);
            }else{
                printf("Command ERROR try again\n");
            }

        }
        //waiting for state send alive
    }
}
void command_send(char *elemId){
    struct TCP send_package = mountTcp(SEND_DATA,elemId);
    send_package_tcp(send_package);
    struct TCP recived_package = recive_package_TCP(m);
}

struct TCP recive_package_TCP(int max_timeout){
    struct timeval tv;
    tv.tv_sec = max_timeout;
    tv.tv_usec = 0;
    struct TCP *recivedPackage = malloc(sizeof (struct TCP));
    //Its not adding correctly the timeout
    if(setsockopt(allSockets.udp_socket,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0){
        printf("ERROR");
    }
    char *buf = malloc(sizeof(struct TCP));
    read(allSockets.tcp_socket,buf,sizeof (struct TCP));
    recivedPackage = (struct TCP *) buf;
    return *recivedPackage;
}

void setup_tcp_socket(){

    struct in_addr* server_addr;


    allSockets.tcp_socket = socket(AF_INET,SOCK_STREAM,0);
    if(allSockets.tcp_socket<0){
        printf("Socket create wrong");
    }

    server_addr = (struct in_addr *)(gethostbyname((const char *)clientConfiguration.server_adress)->h_addr_list[0]);

    memset(&allSockets.tcp_addr_server,0,sizeof (struct sockaddr_in));
    allSockets.tcp_addr_server.sin_family = AF_INET;
    allSockets.tcp_addr_server.sin_addr.s_addr = server_addr->s_addr;
    allSockets.tcp_addr_server.sin_port = htons(atoi(server.tcp_port));

    if(connect(allSockets.tcp_socket,(struct sockaddr *) &allSockets.tcp_addr_server,sizeof (allSockets.tcp_addr_server))<0){
        printf("ERROR connecting tcp socket\n");
        exit(-1);
    }
}
void setup_socket_for_listening(){
    allSockets.tcp_listening_socket = socket(AF_INET,SOCK_STREAM,0);
    if(allSockets.tcp_listening_socket<0){
        printf("Socket create wrong");
    }
    memset(&allSockets.tcp_listening_addr_server,0,sizeof (struct sockaddr_in));
    allSockets.tcp_listening_addr_server.sin_addr.s_addr = htonl(INADDR_ANY); // <-- AQUI FALLA
    allSockets.tcp_listening_addr_server.sin_port = htons(atoi(clientConfiguration.local_TCP_port));
    allSockets.tcp_listening_addr_server.sin_family=AF_INET;

    if(bind(allSockets.tcp_listening_socket,(struct sockaddr *) &allSockets.tcp_listening_addr_server,sizeof (allSockets.tcp_listening_addr_server))<0){
        printf("ERROR binding tcp listening socket\n");
        exit(-1);
    }
}
