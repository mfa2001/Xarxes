//
// Created by mfarr on 23/02/2022.
//
struct ClientConfig{
    char clientID[11];
    char params[5][15]; //Make it tuple
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

struct Sockets{
    int udp_socket;
    struct sockaddr_in udp_addr_server;

    struct sockaddr_in client_adress;

    int tcp_socket;
    struct sockaddr_in tcp_addr_server;
};

struct Server{
    char ServerId[11];
    char ServerCommunication[11];
    char udp_port[4];
    char tcp_port[4];
};

struct Client{
    int unsucssesful_singUps;
};