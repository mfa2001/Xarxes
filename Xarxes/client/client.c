//
// Created by mfarr on 21/02/2022.
//

//Server file
#include <ctype.h>
#include <netinet/in.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "constants.h"
#include "structs.h"
#include "errno.h"
#include "netdb.h"
#include "unistd.h"

//Global variable
struct ClientConfig clientConfiguration;
int client_state;
struct Sockets allSockets;

struct ClientConfig readClientConfig(char filePath[],int debug);
char* splitLast(char* split);
void setupData(int argc, char* argv[]);
void setup_udp_socket();
struct UDP mountPduRequest();
void connection();
void change_client_state(unsigned char newState);
void send_package_udp(struct UDP request);
struct UDP recive_package_UDP();

int main(int argc, char* argv[]){
    setupData(argc,argv);
    //Funtion to print data implements

    //Start with register
    setup_udp_socket();
    connection();

}
void setupData(int argc,char* argv[]){
    int debug = 0;
    char file[50] = "client.cfg";
    for(int index = 1;index<argc;index++){

        if(strcmp(argv[index],"-d")==0){
            debug = 1;
        }else if(strcmp(argv[index],"-c")==0){
            strcpy(file,argv[index+1]);
        }
    }
    clientConfiguration = readClientConfig(file,debug);
}


struct ClientConfig readClientConfig(char filePath[],int debug){
    struct ClientConfig configuration;
    FILE *file = fopen(filePath,"r");
    char line[255];
    char* split;
    size_t len = 255;
    if(file == NULL){
        printf("ERROR reading file");
        exit(-1);
    }
    while(fgets(line,len,file)) {
        char *c = strchr(line, '\n');
        if (c) {
            *c = '\0';
        }
        split = strtok(line, " ");
        if (strcmp(split, "Id") == 0) {
            strcpy(configuration.clientID, splitLast(split));
        } else if (strcmp(split, "Params") == 0) {
            char *param;
            char *lastParams = splitLast(line);
            param = strtok(lastParams, ";");
            int i = 0;
            while (param != NULL) {
                strcpy(configuration.params[i], param);
                param = strtok(NULL, ";");
                i++;
            }

        } else if (strcmp(split, "Local-TCP") == 0) {
            strcpy(configuration.local_TCP_port, splitLast(split));
        } else if (strcmp(split, "Server") == 0) {
            strcpy(configuration.server_adress, splitLast(split));
        } else if (strcmp(split, "Server-UDP") == 0) {
            strcpy(configuration.server_UDP_port, splitLast(split));
        }
    }
    configuration.debug = debug;
    if(configuration.debug ==1){
        printf("Configuration added correctly");
    }
    return configuration;
}

char* splitLast(char* split) {
    for (int i = 0; i <= 1; i++) {
        split = strtok(NULL, " ");
    }
    return split;
}

void setup_udp_socket(){
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
    allSockets.udp_addr_server.sin_port = htons(atoi(clientConfiguration.server_UDP_port));
}

struct UDP mountPduRequest(){
    struct UDP regReq;
    size_t len = sizeof(clientConfiguration.clientID);
    regReq.type = (unsigned char) REG_REQ;
    memccpy(regReq.idTransmissor,clientConfiguration.clientID,0,len);
    strcpy(regReq.idCommunication,"0000000000");
    strcpy(regReq.data,"");
    return regReq;
}

void connection(){
    change_client_state(NOT_REGISTERED);
    struct UDP registerReq = mountPduRequest();
    send_package_udp(registerReq);
    change_client_state(WAIT_ACK_REG);
    struct UDP recivePackage = recive_package_UDP();
    printf("%hhu \n, %s\n, %s\n, %s\n",recivePackage.type,recivePackage.idCommunication,recivePackage.idTransmissor,recivePackage.data);
    close(allSockets.udp_socket);
}

char* get_state_into_str(unsigned char state){
    char *strState;
    if(state == (unsigned char) 0xa0){
        strcpy(strState,"REG_REQ");
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
    }else if(state ==(unsigned char) 0xf6){
        strState="SEND_ALIVE";
    }else{
        strState="ERROR";
    }
    return strState;
}


void change_client_state(unsigned char newState){
    client_state = newState;
    char *newStateStr = get_state_into_str(newState);
    printf("Client state changed to: %s\n",newStateStr);
}

void send_package_udp(struct UDP request){
    int a = sendto(allSockets.udp_socket,&request,sizeof(request),0,
                   (struct sockaddr*)&allSockets.udp_addr_server,sizeof(allSockets.udp_addr_server));
    char message[61];
    if(a<0){
        printf("ERROR SENDING PACKAGE TO UDP\n");
        exit(-1);
    }else if(clientConfiguration.debug ==1){
        printf("Data sended correctly\n");
    }
}

struct UDP recive_package_UDP(){
    struct UDP recivedPackage;
    char* buff;
    int a = recvfrom(allSockets.udp_socket,buff,sizeof(recivedPackage),0,
                         (struct sockaddr*)0,(socklen_t *)0);
    if(a<0) {
        printf("ERROR Cannot recive a message");
        exit(-1);
    }
    return recivedPackage;
}

