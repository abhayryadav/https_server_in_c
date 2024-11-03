#ifndef Serverdec_h
#define Serverdec_h
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct server
{
    int domain;
    int service;
    int protocol;
    int interface;
    int port;
    int backlog;
    int servers_fd;
    struct sockaddr_in address;
    void (*start)(struct server* server);
   
}Server;


Server Server_Constructor(int domain,int service,int protocol,int interface,int port,int backlog, void (*start)(Server* server));
#endif // Serverdec_h