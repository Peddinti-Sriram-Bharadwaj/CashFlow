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

void write_message(const char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

int main(int argc, char *argv[]) {
    char CustomerActionsPath[256];
    snprintf(CustomerActionsPath, sizeof(CustomerActionsPath), "%s%s", basePath, "/customer/customer.out");
    char *username = argv[0];  // Use argv[0] as the username
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "This is the username: %s\n", username);
    write_message(message);

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
    strncpy(operation.username, username, sizeof(operation.username) - 1);
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
    num_bytes = read(sockfd, &passbook, sizeof(passbook));
    if (num_bytes == -1) {
        perror("read");
        close(sockfd);
        exit(1);
    }

    // Display the transaction history
    if (passbook.num_transactions == 0) {
        snprintf(message, sizeof(message), "No transactions found for user %s.\n", username);
        write_message(message);
    } else {
        snprintf(message, sizeof(message), "Transaction History for user %s:\n", username);
        write_message(message);
        write_message("========================================\n");
        for (int i = 0; i < passbook.num_transactions; i++) {
            snprintf(message, sizeof(message), "Transaction %d:\n", i + 1);
            write_message(message);
            snprintf(message, sizeof(message), "  Type: %s\n", passbook.transactions[i].type);
            write_message(message);
            snprintf(message, sizeof(message), "  Amount: %d\n", passbook.transactions[i].amount);
            write_message(message);
            snprintf(message, sizeof(message), "  Date: %s\n", passbook.transactions[i].date);
            write_message(message);
            if (strcmp(passbook.transactions[i].type, "Deposit") != 0 && strcmp(passbook.transactions[i].type, "Withdrawal") != 0) {
                snprintf(message, sizeof(message), "  From: %s\n", passbook.transactions[i].from_username);
                write_message(message);
                snprintf(message, sizeof(message), "  To: %s\n", passbook.transactions[i].to_username);
                write_message(message);
            }
            write_message("----------------------------------------\n");
        }
        write_message("========================================\n");
    }

    // Close the socket
    close(sockfd);

    execvp(CustomerActionsPath, argv);  // Return to the customer actions menu

    return 0; // Exit successfully
}
