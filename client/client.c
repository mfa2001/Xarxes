//
// Created by mfarr on 21/02/2022.
//

//Server file
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "constants.h"
#include "structs.h"
#include "errno.h"

struct ClientConfig configuration;
struct ClientConfig readClientConfig(char filePath[],int debug);
char* splitLast(char* line);
void setupData(int argc, char* argv[]);

void main(int argc, char* argv[]){
    setupData(argc,argv);
}
void setupData(int argc,char* argv[]){
    int debug = 0;
    char file[50] = "..\\client.cfg";
    for(int index = 1;index<argc;index++){

        if(strcmp(argv[index],"-d")==0){
            debug = 1;
        }else if(strcmp(argv[index],"-c")==0){
            strcpy(file,argv[index+1]);
        }
    }
    configuration = readClientConfig(file,debug);
}


struct ClientConfig readClientConfig(char filePath[],int debug){
    FILE* file = fopen(filePath,"r");
    size_t len = 255;
    char* spl;
    char line[255];
    if(file == NULL){
        fprintf(stderr,"File can't be opened");
    }
    while(fgets(line,len,file)){
        spl = strtok(line," ");
        if(strcmp(spl,"Id")==0){
            char* splL = splitLast(spl);
            strncpy(configuration.clientID,splL,len);
        }
        else if(strcmp(spl,"Params")==0){
            char* splL = splitLast(spl);
            char* param = strtok(splL,";");
            int i = 0;
            while(param!=NULL) {
                strncpy(configuration.params[i], param, len);
                i = i+1;
                param = strtok(NULL,";");
            }
        }
        else if(strcmp(spl,"Local-TCP")==0){
            char* splL = splitLast(spl);
            int intSpl = atoi(splL);
            configuration.local_TCP = intSpl;
        }
        else if(strcmp(spl,"Server\0")==0){
            char* splL = splitLast(spl);
            strncpy(configuration.server,splL,len);
        }
        else if(strcmp(spl,"Server-UDP\0")==0){
            char* splL = splitLast(spl);
            int intSpl = atoi(splL);
            configuration.server_UDP = intSpl;
        }
    }
    configuration.debug = debug;
    fclose(file);
    if(configuration.debug==1){
        printf("Configuration added correctly\n");
    }
    return configuration;
}
char* splitLast(char* line){
    for(int i = 0; i <= 1;i++){
        line= strtok(NULL," ");
    }
    return line;
}

