#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
    int * client_fdth;
    int xth;
} thread_args;

void *handle_connection(void *thread_args_nth);	


int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);


	int server_fd;
	//
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// setting SO_REUSEADDR
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
	
    int x =0;
	while (1)
	{
        int client_addr_len;
	    struct sockaddr_in client_addr;
        client_addr_len = sizeof(client_addr);
		int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue; // Go to the next iteration if accept fails
        }
        printf("Client connected\n");

        // Allocate and initialize thread arguments
        thread_args *thread_argsn = malloc(sizeof(thread_args));
        thread_argsn->client_fdth = malloc(sizeof(int));
        *(thread_argsn->client_fdth) = client_fd; // Store the client_fd
        thread_argsn->xth = x++;

        // Create a new thread to handle the connection
        pthread_t new_thread;
        if (pthread_create(&new_thread, NULL, handle_connection, (void *)thread_argsn) != 0) {
            printf("Thread creation failed: %s\n", strerror(errno));
            close(client_fd); // Close client_fd if thread creation fails
            free(thread_argsn->client_fdth);
            free(thread_argsn);
        } else {
            pthread_detach(new_thread); // Detach the thread to let it run independently
        }
	}
    
    
	close(server_fd);

	return 0;
}
void *handle_connection(void *thread_args_nth){

    thread_args *thread_argss = (thread_args *)thread_args_nth;

    int client_fd = *(thread_argss->client_fdth);
    // reading from client_fd
    char readbuffer[1024];
		ssize_t bytes_received = recv(client_fd, readbuffer, sizeof(readbuffer), 0);
		if (bytes_received < 0) {
			printf("Reading error\n");
			close(client_fd);
			return NULL;
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
        if(thread_argss->xth==0){
            sleep(10);
        }
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
			return NULL ;
		}
        puts("-----------");
        return NULL;

}
