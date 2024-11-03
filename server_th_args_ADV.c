#include "serverdec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#define QUEUE_SIZE 5

void *handle_connection(void *thread_args_nth);	

typedef struct {
    int * client_fdth;
    int xth;
} thread_args;

void *handle_connection(void *thread_args_nth){

    thread_args *thread_argss = (thread_args *)thread_args_nth;

    int client_fd = *(thread_argss->client_fdth);
    // reading from client_fd
    char readbuffer[1024];
		ssize_t bytes_received = recv(client_fd, readbuffer, sizeof(readbuffer), 0);
        if (bytes_received < 0) {
            printf("Reading error: %s\n", strerror(errno)); // Added error description
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
        free(thread_argss->client_fdth);
        free(thread_argss);
        close(client_fd); // Close the client socket after handling

 
        return NULL;

}



void start(Server * server) {
    printf("Server started on port %d, waiting for connections...\n", server->port);
    
    int x =0;
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(server->servers_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd == -1) {
            perror("Failed to accept connection");
            continue;
        }
        printf("Client connected\n");




        thread_args *thread_argsn = malloc(sizeof(thread_args));
        if (thread_argsn == NULL) {
            perror("Failed to allocate memory for thread arguments");
            close(client_fd); // Close client_fd if allocation fails
            continue;
        }
        
        // Allocate space for the client file descriptor and store the value
        thread_argsn->client_fdth = malloc(sizeof(int));
        if (thread_argsn->client_fdth == NULL) {
            perror("Failed to allocate memory for client file descriptor");
            free(thread_argsn); // Free the thread_args structure
            close(client_fd); // Close client_fd if allocation fails
            continue;
        }
        *(thread_argsn->client_fdth) = client_fd; // Store the current client_fd
        thread_argsn->xth = x; // Set the desired value of x here
        x++;

        // Create a new thread to handle the connection
        pthread_t new_thread;
        if (pthread_create(&new_thread, NULL, handle_connection, (void *)thread_argsn) != 0) {
            printf("Thread creation failed: %s\n", strerror(errno));
            close(client_fd); // Close client_fd if thread creation fails
            free(thread_argsn->client_fdth); // Free the allocated client_fd space
            free(thread_argsn); // Free the thread_args structure
        } else {
            pthread_detach(new_thread); // Detach the thread to allow independent execution
        }

        // Do not call pthread_join here
        // Close client_fd only within the thread after processing the connection
    }

    close(server->servers_fd); // Close server socket when done
    return;
}









int main() {
    setbuf(stdout, NULL);
 	setbuf(stderr, NULL);
    Server server = Server_Constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 5, start);
    server.start(&server);
    
    return 0;
}
