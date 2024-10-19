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
        char username[20]; // For withdrawal operation (username)
    } data;
};

int main(int argc, char *argv[]) {

    char *username = argv[0]; // Use argv[1] as the username
    char CustomerActionsPath[256];
    snprintf(CustomerActionsPath, sizeof(CustomerActionsPath), "%s%s", basePath, "/customer/customer.out");

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

    // Prepare the operation for withdrawing money
    strcpy(operation.operation, "withdrawMoney");
    
    // Copy username into operation data
    strncpy(operation.data.username, username, sizeof(operation.data.username) - 1);
    operation.data.username[sizeof(operation.data.username) - 1] = '\0'; // Null-terminate

    // Send withdrawal money request to server
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

    // Check if server requests the withdrawal amount
    if (strcmp(server_message, "amount") == 0) {
        // Prompt the user for the withdrawal amount
        int withdrawal_amount;
        printf("Enter the withdrawal amount: ");
        scanf("%d", &withdrawal_amount);

        // Send the withdrawal amount to the server
        num_bytes = send(sockfd, &withdrawal_amount, sizeof(withdrawal_amount), 0);
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
            printf("Withdrawal successful for user %s.\n", operation.data.username);
        } else if (result == -1) {
            printf("Withdrawal failed for user %s.\n", operation.data.username);
        } else {
            printf("Unexpected response from server.\n");
        }
    } else {
        printf("Unexpected message from server: %s\n", server_message);
    }

    // Close the socket
    close(sockfd);

    // Execute the next customer action
    if (execvp(CustomerActionsPath, argv) == -1) {
        perror("execvp failed");
        exit(1);
    }

    return 0; // This line will not be reached if execvp is successful
}
