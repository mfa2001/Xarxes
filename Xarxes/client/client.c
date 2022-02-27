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

//Global variable
struct ClientConfig clientConfiguration;
int client_state;

struct ClientConfig readClientConfig(char filePath[],int debug);
char* splitLast(char* line);
void setupData(int argc, char* argv[]);
void substring(char destination[], char source[], int start, int final);
void setup_udp_socket();
struct UDP mountPDU(unsigned char type,char idComunication[],char data[]);
void connection();
void change_client_state(unsigned char newState);

void main(int argc, char* argv[]){
    setupData(argc,argv);
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
    size_t len = 255;
    char* spl;
    char line[255];
    if(file == NULL){
        fprintf(stderr,"File can't be opened");
        exit(-1);
    }
    while(fgets(line,len,file)){
        spl = strtok(line," ");
        if(strcmp(spl,"Id")==0){
            substring(configuration.clientID,line,5,5+12);
        }
        else if(strcmp(spl,"Params")==0){
            char *param;
            char *lastParams = splitLast(line);
            param = strtok(lastParams,";");
            int i = 0;
            while(param!=NULL){
                strcpy(configuration.params[i],param);
                param = strtok(NULL,";");
                i++;
            }
        }
        else if(strcmp(spl,"Local-TCP")==0){
            substring(configuration.local_TCP_port,line,12,12+4);
        }
        else if(strcmp(spl,"Server")==0){
            substring(configuration.server_adress,line,9,9+20);
        }
        else if(strcmp(spl,"Server-UDP")==0){
            substring(configuration.server_UDP_port,line,13,13+4);
        }
    }
    configuration.debug = debug;
    fclose(file);
    if(configuration.debug==1){
        printf("Configuration added correctly\n");
    }
    return configuration;
}
void substring(char destination[], char source[], int start, int final){
    int index = 0;
    while(index < final - start){
        if(strcmp(&source[start+index],"\n")==0){
            break;
        }
        if(isalpha(source[start+index]) || isdigit(source[start+index])){
            destination[index] = source[start+index];
            index++;
        }
        else{
            start++;
        }
    }
    destination[start+index] = '\0';
}

char* splitLast(char* line){
    for(int i = 0; i <= 1;i++){
        line= strtok(NULL," ");
    }
    return line;
}

void setup_udp_socket(){
    //create sockets

    struct hostent *ent;
    int udpSocket, udpPort;
    struct sockaddr_in clientAddr, serverAddr;

    ent = gethostbyname(clientConfiguration.server_adress);
    if(!ent){
        printf("ERROR, can not take host name");
        exit(-1);
    }

    udpSocket = socket(AF_INET, SOCK_DGRAM,0);
    if(udpSocket < 0){
        printf("ERROR");
        exit(-1);
    }

    memset(&clientAddr,0,sizeof(struct sockaddr_in));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(0);

    if(bind(udpSocket,(struct sockaddr *)&clientAddr,sizeof(struct sockaddr_in))<0){
        printf("ERROR");
        exit(-1);
    }

    memset(&serverAddr,0,sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr=(((struct in_addr *) ent->h_addr_list[0])->s_addr);
    serverAddr.sin_port = htons(atoi(clientConfiguration.server_UDP_port));
}

struct UDP mountPDU(unsigned char type,char idComunication[],char data[]){
    struct UDP regReq;
    regReq.type = type;
    strcpy(regReq.idTransmissor,clientConfiguration.clientID);
    strcpy(regReq.idCommunication,idComunication);
    strcpy(regReq.data,data);
    return regReq;

}

void connection(){
    change_client_state(NOT_REGISTERED);
    unsigned char type = REG_REQ;
    struct UDP registerReq = mountPDU(type,"0000000000","");
}

char* get_state_into_str(unsigned char state){
    char *strState;
    unsigned char try = (unsigned char) 0xa0;
    if(state == (unsigned char) 0xa0){
        strState="REG_REQ";
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
    printf("Client state changed to: %s",newStateStr);
}

