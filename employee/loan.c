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
        char username[20]; // For employee's username
    } data;
};

struct LoanApprovalRequest {
    char username[20];  // Customer's username
    int amount;         // Amount of the loan
};

int main(int argc, char *argv[]) {
    printf("here\n");
    char EmployeeActionsPath[256];
    snprintf(EmployeeActionsPath, sizeof(EmployeeActionsPath), "%s%s", basePath, "/employee/employee.out");
    int sockfd;
    struct sockaddr_un server_addr;
    struct Operation operation; // To hold operation details
    
    // Get the employee username from command-line arguments
    strcpy(operation.data.username, argv[0]);
    strcpy(operation.operation, "loanApproval"); // Define the operation
    printf("%s\n",operation.data.username);

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

    // Step 1: Send the operation to the server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Step 2: Wait for "username" message from server
    char request[20];
    recv(sockfd, request, sizeof(request), 0);
    
    if (strcmp(request, "username") == 0) {
        // Step 3: Send the customer's username
        char customer_username[20];
        printf("Enter the username of the customer whose loan to process: ");
        scanf("%s", customer_username);
        send(sockfd, customer_username, sizeof(customer_username), 0);

        // Step 4: Receive loan details
        struct LoanApprovalRequest loan_request_info;
        recv(sockfd, &loan_request_info, sizeof(loan_request_info), 0);

        // Display loan details
        printf("Processing loan for %s, Amount: %d\n", loan_request_info.username, loan_request_info.amount);

        // Step 5: Send response back to the server
        char response[20];
        printf("Enter response (approve/reject): ");
        scanf("%s", response);
        send(sockfd, response, sizeof(response), 0);
    }

    // Close the socket
    close(sockfd);
    execvp(EmployeeActionsPath, argv);
    return 0;
}