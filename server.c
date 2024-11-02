#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	//
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	//
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};

	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
	while (1)
	{
		int client_fd  = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		printf("Client connected\n");
		
		// reading from client_fd
		char readbuffer[1024];
		ssize_t bytes_received = recv(client_fd, readbuffer, sizeof(readbuffer), 0);
		if (bytes_received < 0) {
			printf("Reading error\n");
			close(client_fd);
			close(server_fd);
			return 1;
		}
		printf("%s",readbuffer);
		// extracting the path
		readbuffer[bytes_received] = '\0';
		char * reqPath = strtok(readbuffer," ");
		reqPath = strtok(NULL," ");
		
		char reply[2048] = {0};
		char extracted[1024] = {0}; // Buffer to hold the extracted text
		int bytes_sent =0;
		// Check if the request path starts with "/echo/"
		if (reqPath && strncmp(reqPath, "/echo/", 6) == 0) {
			strncpy(extracted, reqPath + 6, sizeof(extracted) - 1);
    		printf("Extracted: %s\n", extracted);
			
			
			sprintf(reply,
            "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
            "%ld\r\n\r\n%s",
            strlen(extracted), extracted);
			bytes_sent = send(client_fd, reply, strlen(reply), 0);
		}
		// Check if the request path starts with "/user-agent/"
		else if (reqPath && strncmp(reqPath, "/user-agent/", 12) == 0) {

			strtok(0, "\r\n");
			strtok(0, "\r\n");
			char *userAgent = strtok(0, "\r\n") + 12;
			const char *format = "HTTP/1.1 200 OK\r\nContent-Type: "
								"text/plain\r\nContent-Length: %zu\r\n\r\n%s";
			char response[1024];
			sprintf(response, format, strlen(userAgent), userAgent);
			send(client_fd, response, sizeof(response), 0);
		} 
		else {
			char *reply = "HTTP/1.1 404 Not Found\r\n\r\n";
			bytes_sent = send(client_fd, reply, strlen(reply), 0);
		}

		if(bytes_sent<0){
			printf("send failed");
			return 1;
		}
	}
		
	close(server_fd);

	return 0;
}
