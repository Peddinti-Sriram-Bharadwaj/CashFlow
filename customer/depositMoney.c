#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include "../global.c"

#define SOCKET_PATH "/tmp/server"
#define BUFFER_SIZE 256

struct Operation {
    char operation[20];
    union {
        char username[20]; // For deposit operation (username)
    } data;
};

void write_message(int fd, const char *message) {
    write(fd, message, strlen(message));
}

void read_input(int fd, char *buffer, size_t size) {
    ssize_t num_bytes = read(fd, buffer, size - 1);
    if (num_bytes > 0) {
        buffer[num_bytes] = '\0';
    }
}

int main(int argc, char *argv[]) {
    char CustomerActionsPath[256];
    snprintf(CustomerActionsPath, sizeof(CustomerActionsPath), "%s%s", basePath, "/customer/customer.out");

    char *username = argv[0]; // Use argv[0] as the username
    write_message(STDOUT_FILENO, "This is the username: ");
    write_message(STDOUT_FILENO, username);
    write_message(STDOUT_FILENO, "\n");

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

    // Prepare the operation for deposit money
    strcpy(operation.operation, "depositMoney");
    
    // Copy username into operation data
    strncpy(operation.data.username, username, sizeof(operation.data.username) - 1);
    operation.data.username[sizeof(operation.data.username) - 1] = '\0'; // Null-terminate

    // Send deposit money request to server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Receive the "amount" message from the server
    char server_message[BUFFER_SIZE];
    num_bytes = recv(sockfd, server_message, sizeof(server_message) - 1, 0);
    if (num_bytes == -1) {
        perror("recv");
        close(sockfd);
        exit(1);
    }
    server_message[num_bytes] = '\0'; // Null-terminate

    // Check if server requests the deposit amount
    if (strcmp(server_message, "amount") == 0) {
        // Prompt the user for the deposit amount
        int deposit_amount;
        write_message(STDOUT_FILENO, "Enter the deposit amount: ");
        
        char input_buffer[BUFFER_SIZE];
        read_input(STDIN_FILENO, input_buffer, sizeof(input_buffer));
        deposit_amount = atoi(input_buffer);

        // Send the deposit amount to the server
        num_bytes = send(sockfd, &deposit_amount, sizeof(deposit_amount), 0);
        if (num_bytes == -1) {
            perror("send");
            close(sockfd);
            exit(1);
        }

        // Receive the result from the server (1 for success, -1 for failure)
        int result;
        num_bytes = recv(sockfd, &result, sizeof(result), 0);
        if (num_bytes == -1) {
            perror("recv");
            close(sockfd);
            exit(1);
        }

        // Print appropriate message based on server response
        if (result == 1) {
            write_message(STDOUT_FILENO, "Deposit successful for user ");
            write_message(STDOUT_FILENO, operation.data.username);
            write_message(STDOUT_FILENO, ".\n");
        } else if (result == -1) {
            write_message(STDOUT_FILENO, "Deposit failed for user ");
            write_message(STDOUT_FILENO, operation.data.username);
            write_message(STDOUT_FILENO, ".\n");
        } else {
            write_message(STDOUT_FILENO, "Unexpected response from server.\n");
        }
    } else {
        write_message(STDOUT_FILENO, "Unexpected message from server: ");
        write_message(STDOUT_FILENO, server_message);
        write_message(STDOUT_FILENO, "\n");
    }

    // Close the socket
    close(sockfd);

    // Execute the next customer action
    execvp(CustomerActionsPath, argv);
    perror("execvp failed");

    return 0; // Exit successfully
}
