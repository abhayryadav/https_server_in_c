#include "serverdec.h"
#include <stdio.h>
#include <stdlib.h>


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>


Server Server_Constructor(int domain, int service, int protocol, int interface, int port, int backlog, void (*start)(struct server* server)) {
    Server server;

    server.domain=domain;
    server.service=service;
    server.protocol=protocol;
    server.interface=interface;
    server.port=port;
    server.backlog=backlog;
    server.start = start;


    server.address.sin_family = domain;
    server.address.sin_port=htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.servers_fd = socket(domain,service,protocol);
                                //AF_INET,SOCK_STREAM,0
    if(server.servers_fd == -1){
        perror("Failed to connect");
        exit(1);
    }
	
	// setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors


	int reuse = 1;
	if (setsockopt(server.servers_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("Setting Sockoptions failed: %s \n", strerror(errno));
		exit(1);
	}

	if (bind(server.servers_fd, (struct sockaddr *) &server.address, sizeof(server.address)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		exit(1);
	}
	
	if (listen(server.servers_fd, server.backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		exit(1);
	}
	
	// printf("Waiting for a client to connect...\n");
    
    return server;
}