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

void write_string(int fd, const char *str) {
    write(fd, str, strlen(str));
}

void read_string(int fd, char *buffer, size_t size) {
    ssize_t bytes_read = read(fd, buffer, size - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
    }
}

ssize_t read_full(int sockfd, void *buf, size_t len) {
    size_t total_bytes_read = 0;
    ssize_t bytes_read;
    char *buffer = (char *)buf;

    while (total_bytes_read < len) {
        bytes_read = read(sockfd, buffer + total_bytes_read, len - total_bytes_read);
        if (bytes_read <= 0) {
            break; // Error or end of data
        }
        total_bytes_read += bytes_read;
    }
    return total_bytes_read;
}

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

int main(int argc, char *argv[]) {
    if (argc < 1) {
        write_string(STDERR_FILENO, "Usage: ");
        write_string(STDERR_FILENO, argv[0]);
        write_string(STDERR_FILENO, " <employee_username>\n");
        exit(1);
    }

    char EmployeeActionsPath[256];
    snprintf(EmployeeActionsPath, sizeof(EmployeeActionsPath), "%s%s", basePath, "/employee/employee.out");
    
    char *employee_username = argv[0];  // Use argv[0] as the employee username
    write_string(STDOUT_FILENO, "Employee: ");
    write_string(STDOUT_FILENO, employee_username);
    write_string(STDOUT_FILENO, "\n");

    // Ask the user for the customer whose passbook they want to view
    char customer_username[20];
    write_string(STDOUT_FILENO, "Enter the username of the customer whose passbook you want to view: ");
    read_string(STDIN_FILENO, customer_username, sizeof(customer_username));
    remove_newline(customer_username);

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
    num_bytes = read_full(sockfd, &passbook, sizeof(passbook)); // Use read_full to ensure full struct is read
    if (num_bytes != sizeof(passbook)) {
        write_string(STDOUT_FILENO, "Failed to read the complete passbook\n");
        close(sockfd);
        exit(1);
    }

    // Display the transaction history
    if (passbook.num_transactions == 0) {
        write_string(STDOUT_FILENO, "No transactions found for user ");
        write_string(STDOUT_FILENO, customer_username);
        write_string(STDOUT_FILENO, ".\n");
    } else {
        write_string(STDOUT_FILENO, "========================================\n");
        write_string(STDOUT_FILENO, "Transaction History for user ");
        write_string(STDOUT_FILENO, customer_username);
        write_string(STDOUT_FILENO, ":\n");
        write_string(STDOUT_FILENO, "========================================\n");
        for (int i = 0; i < passbook.num_transactions; i++) {
            char buffer[BUFFER_SIZE];
            snprintf(buffer, sizeof(buffer), "Transaction %d:\n  Type: %s\n  Amount: %d\n  Date: %s\n", 
                     i + 1, passbook.transactions[i].type, passbook.transactions[i].amount, passbook.transactions[i].date);
            write_string(STDOUT_FILENO, buffer);
            if (strcmp(passbook.transactions[i].type, "Transfer") == 0) {
                snprintf(buffer, sizeof(buffer), "  From: %s\n  To: %s\n", 
                         passbook.transactions[i].from_username, passbook.transactions[i].to_username);
                write_string(STDOUT_FILENO, buffer);
            }
            write_string(STDOUT_FILENO, "----------------------------------------\n");
        }
    }

    // Close the socket
    close(sockfd);

    // Return to the employee actions menu
    execvp(EmployeeActionsPath, argv);

    return 0; // Exit successfully
}
