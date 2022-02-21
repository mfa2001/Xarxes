//
// Created by mfarr on 21/02/2022.
//

//Server file
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct ClientConfig;
void readClientConfig(struct ClientConfig configuration, char* filePath,int debug)

void main(){
    char file[] = "ClientConfigFile";

}
struct ClientConfig{
    int udp_port;
    int tcp_port;
    char ID[];
};

void readClientConfig(struct ClientConfig configuration,char* filePath,int debug){
    printf("hola")
}