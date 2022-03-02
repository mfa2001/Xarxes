//
// Created by mfarr on 23/02/2022.
//
struct ClientConfig{
    char clientID[10];
    char params[5][15]; //Make it tuple
    char local_TCP_port[4];
    char server_adress[20];
    char server_UDP_port[4];
    int debug;
};

struct UDP{
    unsigned char type;
    char idTransmissor[11];
    char idCommunication[11];
    char data[61];
};

struct Sockets{
    int udp_socket;
    struct sockaddr_in udp_addr_server;

    struct sockaddr_in client_adress;

    int tcp_socket;
    struct sockaddr_in tcp_addr_server;
};