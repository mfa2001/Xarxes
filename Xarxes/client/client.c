//
// Created by mfarr on 21/02/2022.
//

//Server file
#include <netinet/in.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "constants.h"
#include "structs.h"
#include "netdb.h"
#include "unistd.h"

#define LONGDADES 100

//Global variable
struct ClientConfig clientConfiguration;
char *client_state = NULL;
struct Sockets allSockets;
struct Client clientInfo;

void readClientConfig(FILE *file,int debug);
char* splitLast(char split[]);
void setupData(int argc, char* argv[]);
void setup_udp_socket();
struct UDP mountPduRequest();
void connection();
void change_client_state(char *newState);
void send_package_udp(struct UDP request);
struct UDP recive_package_UDP();

int main(int argc, char* argv[]){
    setupData(argc,argv);
    //Funtion to print data implements

    //Start with register
    setup_udp_socket();
    change_client_state("NOT_REGISTERED");

    connection();

}
void setupData(int argc,char* argv[]){
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
            char *token = splitLast(split);
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
/*
unsigned char get_state_into_unChar(char *state){
    unsigned char state_type;
    if(strcmp(state,"REG_REQ")==0){
        state_type = (unsigned char) 0xa0;
    }else if(strcmp(state,"REG_ACK")==0){
        state_type = (unsigned char) 0xa1;
    }
}
*/
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
    }else if(state ==(unsigned char) 0xf6){
        strState="SEND_ALIVE";
    }else{
        strState="ERROR";
    }
    return strState;
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
    memcpy(regReq.idTransmissor,clientConfiguration.clientID,len);
    strcpy(regReq.idCommunication,"0000000000");
    strcpy(regReq.data,"");
    return regReq;
}

void connection(){
    /*clientInfo.unsucssesful_singUps = 0;
    while(clientInfo.unsucssesful_singUps < o){
        if(clientConfiguration.debug == 1){
            //Escribir nuevo intento + numero de trys
        }
        for (int reg_sent = 0; reg_sent < n; reg_sent++){

            if(get_state_into_str(recivePackage.type) == "REG_REJ"){

            }
        }
    }
     */
    struct UDP registerReq;
    registerReq = mountPduRequest();
    send_package_udp(registerReq);
    change_client_state("WAIT_ACK_REG");
    struct UDP recivePackage = recive_package_UDP();
    close(allSockets.udp_socket);
    exit(-1);
}

void change_client_state(char *newState){
    client_state = malloc(sizeof(&newState));
    strcpy(client_state,newState);
    printf("Client state changed to: %s\n",client_state);
}

void send_package_udp(struct UDP request){
    int a = sendto(allSockets.udp_socket,&request,sizeof(request),0,
                   (struct sockaddr*)&allSockets.udp_addr_server,sizeof(allSockets.udp_addr_server));
    //char message[61];
    if(a<0){
        printf("ERROR SENDING PACKAGE TO UDP\n");
        exit(-1);
    }else if(clientConfiguration.debug ==1){
        printf("Data sended correctly\n");
    }
}

struct UDP recive_package_UDP(){
    struct UDP *recivedPackage = malloc(sizeof (struct UDP));
    char *buf = malloc(sizeof(struct UDP));
    int a = recvfrom(allSockets.udp_socket,buf,sizeof(struct UDP),0,
                         (struct sockaddr *)0,(socklen_t *)0);
    if(a<0) {
        printf("ERROR Cannot recive a message");
        exit(-1);
    }
    recivedPackage = (struct UDP *) buf;
    return *recivedPackage;
}

