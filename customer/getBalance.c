#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../global.c"

#define SOCKET_PATH "/tmp/server"
#define BUFFER_SIZE 256

struct Operation {
    char operation[20];
    union {
        char username[20]; // For balance retrieval
    } data;
};

int main(int argc, char *argv[]) {
    char CustomerActionsPath[256];
    snprintf(CustomerActionsPath, sizeof(CustomerActionsPath), "%s%s", basePath, "/customer/customer.out");


    char *username = argv[0]; // Use the second argument as the username
    printf("This is the username: %s\n", username);
    fflush(stdout); // Ensure output is printed immediately

    int sockfd;
    struct sockaddr_un server_addr;
    struct Operation operation;

    // Create a stream socket
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    // Prepare the operation to get balance for a specific user
    strcpy(operation.operation, "getbalance");
    
    // Copy username into operation data
    strncpy(operation.data.username, username, sizeof(operation.data.username) - 1);
    operation.data.username[sizeof(operation.data.username) - 1] = '\0'; // Null-terminate

    // Send request to server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);

    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Receive the response from the server (balance or error)
    int balance;
    
    num_bytes = recv(sockfd, &balance, sizeof(balance), 0);
    
    if (num_bytes == -1) {
        perror("recv");
        close(sockfd);
        exit(1);
    }

    // Process the balance or user not found error
    if (balance == -1) {
        printf("User %s not found.\n", operation.data.username);
        fflush(stdout); // Ensure output is printed immediately
    } else {
        printf("Balance for user %s: %d\n", operation.data.username, balance);
        fflush(stdout); // Ensure output is printed immediately
    }

    // Close the socket
    close(sockfd);
        execvp(CustomerActionsPath, argv);
        perror("execvp failed");

    return 0; // Exit successfully
}
