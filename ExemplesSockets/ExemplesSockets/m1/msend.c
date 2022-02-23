/*
 */


#define MULTICAST

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#define DEFAULT_GROUP	0xe0027fff
#define DEFAULT_PORT		9876
#define MAXPDU 4096
#define WIDTH 16

u_long groupaddr = DEFAULT_GROUP;
u_short groupport = DEFAULT_PORT;

int main(int argc, char *argv[])
{
    int i, j;
    int sock, rd, length, origlen;
    char buf[MAXPDU];
    struct sockaddr_in name;
    struct ip_mreq imr;
    char *interface = NULL;
    char debug=0;

    char tmpstr[100];
    int ttl;
    int port1, port2;
    u_long time1, time2;
    struct in_addr source;

    if (argc > 4) {
	printf("usage: %s [group [port [interface]]]\n", argv[0]);
	exit(1);
    }

    if (argc > 1) {
	groupaddr = inet_addr(argv[1]);
    }

    if (argc > 2) {
	groupport = (u_short)atoi(argv[2]);
    }

    if (argc == 4) {
	interface=argv[3];
    }    

    if((sock=socket( AF_INET, SOCK_DGRAM, 0 )) < 0) {
        perror("socket");
        exit(1);
    }

    imr.imr_multiaddr.s_addr = groupaddr;
    imr.imr_multiaddr.s_addr = htonl(imr.imr_multiaddr.s_addr);
    if (interface!=NULL) {
        imr.imr_interface.s_addr = inet_addr(interface);
    } else {
	imr.imr_interface.s_addr = htonl(INADDR_ANY);
    }
    imr.imr_interface.s_addr = htonl(imr.imr_interface.s_addr);

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
	&imr, sizeof(struct ip_mreq)) < 0 ) {
	perror("setsockopt - IP_ADD_MEMBERSHIP");
	exit(1);
    }

	/*
	 *	Use INADDR_ANY if your multicast port doesn't allow
	 *	binding to a multicast address.
	 *
	 */

    name.sin_family = AF_INET;
#ifndef CANT_MCAST_BIND
    name.sin_addr.s_addr = htonl(groupaddr);
#else
    name.sin_addr.s_addr = INADDR_ANY;
#endif
    name.sin_port = htons(groupport);
    /*
    if (bind(sock, &name, sizeof(name))) {
	perror("bind");
	exit(1);
    }
    */

    if(connect(sock,&name,sizeof(name))<0)
    {
    	perror("connect");
	exit(0);
    }
    strcpy(buf,"TEST");
    if ((length = send(sock, (char *) buf, sizeof(buf), 0)) < 0) 
    {
		perror("send");
		exit(1);
    }
    close(sock);
    return(0);
}
