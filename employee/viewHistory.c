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

// Structure for the operation the client sends
struct Operation {
    char operation[20]; // Operation type (viewTransactionHistory)
    char username[20];  // Username of the customer
};

// Transaction structure for each transaction (Deposit, Withdrawal, Transfer)
struct Transaction {
    char type[20];      // Type of transaction (Deposit, Withdrawal, Transfer)
    int amount;         // Amount for the transaction
    char date[20];      // Date of the transaction
    char from_username[20];  // For Transfers: Username of the sender
    char to_username[20];    // For Transfers: Username of the recipient
};

// Passbook structure containing transaction history and username
struct Passbook {
    char username[20];           // Username of the customer
    int num_transactions;        // Number of transactions in the passbook
    struct Transaction transactions[100];  // Array to store the transactions
};

int main(int argc, char *argv[]) {
    if (argc < 1) {
        fprintf(stderr, "Usage: %s <employee_username>\n", argv[0]);
        exit(1);
    }

    char EmployeeActionsPath[256];
    snprintf(EmployeeActionsPath, sizeof(EmployeeActionsPath), "%s%s", basePath, "/employee/employee.out");
    
    char *employee_username = argv[0];  // Use argv[0] as the employee username
    printf("Employee: %s\n", employee_username);
    fflush(stdout);  // Ensure output is printed immediately

    // Ask the user for the customer whose passbook they want to view
    char customer_username[20];
    printf("Enter the username of the customer whose passbook you want to view: ");
    scanf("%19s", customer_username);  // Take input for the customer username

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

    // Prepare the operation for viewing transaction history
    strcpy(operation.operation, "viewHistory");
    strncpy(operation.username, customer_username, sizeof(operation.username) - 1);
    operation.username[sizeof(operation.username) - 1] = '\0';  // Null-terminate

    // Send the operation request to the server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Receive the passbook (transaction history) response from the server
    struct Passbook passbook;
    num_bytes = recv(sockfd, &passbook, sizeof(passbook), 0);
    if (num_bytes == -1) {
        perror("recv");
        close(sockfd);
        exit(1);
    }

    // Display the transaction history
    if (passbook.num_transactions == 0) {
        printf("No transactions found for user %s.\n", customer_username);
    } else {
        printf("Transaction History for user %s:\n", customer_username);
        for (int i = 0; i < passbook.num_transactions; i++) {
            printf("Transaction %d:\n", i + 1);
            printf("  Type: %s\n", passbook.transactions[i].type);
            printf("  Amount: %d\n", passbook.transactions[i].amount);
            printf("  Date: %s\n", passbook.transactions[i].date);
            if (strcmp(passbook.transactions[i].type, "Transfer") == 0) {
                printf("  From: %s\n", passbook.transactions[i].from_username);
                printf("  To: %s\n", passbook.transactions[i].to_username);
            }
            printf("\n");
        }
    }

    // Close the socket
    close(sockfd);

    execvp(EmployeeActionsPath, argv);  // Return to the customer actions menu

    return 0; // Exit successfully
}
