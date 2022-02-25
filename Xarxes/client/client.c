//
// Created by mfarr on 21/02/2022.
//

//Server file
#include <ctype.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "constants.h"
#include "structs.h"
#include "errno.h"

struct ClientConfig clientConfiguration;
struct ClientConfig readClientConfig(char filePath[],int debug);
char* splitLast(char* line);
void setupData(int argc, char* argv[]);
void substring(char destination[], char source[], int start, int final);

void main(int argc, char* argv[]){
    setupData(argc,argv);
    //Start with register
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
            substring(configuration.local_TCP,line,12,12+4);
        }
        else if(strcmp(spl,"Server")==0){
            substring(configuration.server,line,9,9+20);
        }
        else if(strcmp(spl,"Server-UDP")==0){
            substring(configuration.server_UDP,line,13,13+4);
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
    char specialChar[2] = "-;";
    int index = 0;
    while(index < final - start){
        if(isalpha(source[start+index]) || isdigit(source[start+index])){
            destination[index] = source[start+index];
            index++;
        }
        else{
            start++;
        }
    }
    destination[index] = '\0';
}

char* splitLast(char* line){
    for(int i = 0; i <= 1;i++){
        line= strtok(NULL," ");
    }
    return line;
}

