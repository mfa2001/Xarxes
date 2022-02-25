//
// Created by mfarr on 23/02/2022.
//
struct ClientConfig{
    char clientID[10];
    char params[5][15];
    char local_TCP[4];
    char server_UDP[4];
    char server[20];
    int debug;
};

struct UDP{
    unsigned char type;
    char idTransmissor[11];
    char idCommunication[11];
    char data[61];
};