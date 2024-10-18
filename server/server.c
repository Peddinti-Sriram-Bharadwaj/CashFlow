#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    char buffer[BUFFER_SIZE];
    int read_size;

    // Send welcome message
    char *message = "Hi\n";
    send(sock, message, strlen(message), 0);

    // Receive messages from client
    while ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        if (strcmp(buffer, "logout\n") == 0) {
            break;
        }
        printf("Client: %s", buffer);
    }

    // Close the socket and end the thread
    close(sock);
    free(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket, *client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t thread_id;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running and waiting for connections...\n");

    // Accept incoming connections
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0) {
        printf("Connection accepted\n");

        client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)client_socket) < 0) {
            perror("could not create thread");
            free(client_socket);
        }
    }

    if (new_socket < 0) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    close(server_fd);
    return 0;
}