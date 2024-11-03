#ifndef Serverdef_h
#define Serverdef_h
#include <sys/socket.h>
#include <netinet/in.h>

struct server
{
    int domain;
    int service;
    int protocol;
    int interface;
    int port;
    int backlog;
    struct sockaddr_in address;
    int socket;
    void(*start)(void);
};
typedef struct server Server;
Server Server_Constructor(int domain,int service,int protocol,int interface,int port,int backlog, struct sockaddr_in address,int socket,void(*start)(void));
#endif Serverdef_h