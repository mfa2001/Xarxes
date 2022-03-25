//
// Created by mfarr on 23/02/2022.
//
struct ClientConfig{
    char clientID[11];
    char local_TCP_port[5];
    char *server_adress;
    char server_UDP_port[5];
    int debug;
};

struct UDP{
    unsigned char type;
    char idTransmissor[11];
    char idCommunication[11];
    char data[61];
};

struct Server{
    char ServerId[11];
    char ServerCommunication[11];
    char udp_port[4];
    char tcp_port[4];
};

struct Sockets{
    int udp_socket;
    struct sockaddr_in udp_addr_server;

    struct sockaddr_in client_adress;

    int tcp_socket;
    struct sockaddr_in tcp_addr_server;

    int tcp_listening_socket;
    struct sockaddr_in tcp_listening_addr_server;
};

struct Client{
    int unsucssesful_singUps;
};

struct Elements{
    char strElem[15];
    char valElem[16];
};

struct TCP{
    unsigned char type;
    char id_transmisor[11];
    char id_comunicacio[11];
    char element[8];
    char valor[16];
    char info[80];
};