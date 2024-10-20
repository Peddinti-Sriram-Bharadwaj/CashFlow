#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#define SOCKET_PATH "/tmp/server"
#include "../global.c" // Include your global variables and definitions

// Structure for the operation the client sends
struct Operation {
    char operation[20];  // Operation type (getbalance)
    char username[20];   // Username of the customer
};

void write_message(const char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

int main(int argc, char *argv[]) {
    char CustomerActionsPath[256];
    snprintf(CustomerActionsPath, sizeof(CustomerActionsPath), "%s%s", basePath, "/customer/customer.out");

    char *username = argv[0]; // Use the second argument as the username
    write_message("========================================\n");
    write_message("This is the username: ");
    write(STDOUT_FILENO, username, strlen(username));
    write_message("\n");
    write_message("========================================\n");

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
    strncpy(operation.username, username, sizeof(operation.username) - 1);
    operation.username[sizeof(operation.username) - 1] = '\0'; // Null-terminate

    // Send request to server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    write_message("Waiting for balance response...\n");

    // Receive the response from the server (balance or error)
    int balance;
    num_bytes = read(sockfd, &balance, sizeof(balance));
    if (num_bytes == -1) {
        perror("read");
        close(sockfd);
        exit(1);
    }

    // Process the balance or user not found error
    if (balance == -1) {
        write_message("========================================\n");
        write_message("User ");
        write(STDOUT_FILENO, username, strlen(username));
        write_message(" not found.\n");
        write_message("========================================\n");
    } else {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "========================================\nBalance for user %s: %d\n========================================\n", username, balance);
        write_message(buffer);
    }

    // Close the socket
    close(sockfd);
    execvp(CustomerActionsPath, argv); // Execute the customer actions program
    return 0; // Exit successfully
}
