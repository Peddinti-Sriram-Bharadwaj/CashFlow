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
        char username[20]; // For loan application (username)
    } data;
};

int main(int argc, char *argv[]) {
    char CustomerActionsPath[256];
    snprintf(CustomerActionsPath, sizeof(CustomerActionsPath), "%s%s", basePath, "/customer/customer.out");

    char *username = argv[0]; // Use argv[0] as the username
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

    // Prepare the operation to apply for a loan
    strcpy(operation.operation, "loanApplication");
    
    // Copy username into operation data
    strncpy(operation.data.username, username, sizeof(operation.data.username) - 1);
    operation.data.username[sizeof(operation.data.username) - 1] = '\0'; // Null-terminate

    // Send loan application request to server
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

    // Check if server requests the loan amount
    if (strcmp(server_message, "amount") == 0) {
        // Prompt the user for the loan amount
        int loan_amount;
        printf("Enter the loan amount: ");
        scanf("%d", &loan_amount);

        // Send the loan amount to the server
        num_bytes = send(sockfd, &loan_amount, sizeof(loan_amount), 0);
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
            printf("Loan request successful for user %s.\n", operation.data.username);
        } else if (result == -1) {
            printf("Loan request failed for user %s.\n", operation.data.username);
        } else {
            printf("Unexpected response from server.\n");
        }
    } else {
        printf("Unexpected message from server: %s\n", server_message);
    }

    // Close the socket
    close(sockfd);

    // Execute the next customer action
    execvp(CustomerActionsPath, argv);
    perror("execvp failed");

    return 0; // Exit successfully
}