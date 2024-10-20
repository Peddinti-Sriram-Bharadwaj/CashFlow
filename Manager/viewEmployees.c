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

// Structure to represent an Operation
struct Operation {
    char operation[20];
};

// Structure to hold Employee data
struct Employee {
    char username[20]; // Employee's username
    char processing_usernames[5][20]; // Up to 5 usernames of customers whose loans are being processed
};

void write_message(const char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

void write_number(int number) {
    char buffer[20];
    int length = snprintf(buffer, sizeof(buffer), "%d", number);
    write(STDOUT_FILENO, buffer, length);
}

int main(int argc, char *argv[]) {
    char ManagerActionsPath[256];
    snprintf(ManagerActionsPath, sizeof(ManagerActionsPath), "%s%s", basePath, "/manager/manager.out");

    int sockfd;
    struct sockaddr_un server_addr;
    struct Operation operation; // Create an Operation structure

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

    // Prepare the operation to list employees with at least one "None"
    strcpy(operation.operation, "pendingEmployees");

    // Send the operation to the server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Receive the number of employees with "None" in processing_usernames
    int pending_count;
    num_bytes = read(sockfd, &pending_count, sizeof(pending_count));
    if (num_bytes == -1) {
        perror("read");
        close(sockfd);
        exit(1);
    }

    write_message("========================================\n");
    write_message("Number of employees with at least one 'None': ");
    write_number(pending_count);
    write_message("\n");
    write_message("========================================\n");

    // Step 4: Receive each employee and display their username
    for (int i = 0; i < pending_count; i++) {
        struct Employee employee;
        num_bytes = read(sockfd, &employee, sizeof(struct Employee));
        if (num_bytes == -1) {
            perror("read");
            close(sockfd);
            exit(1);
        }

        // Display the employee username
        write_message("----------------------------------------\n");
        write_message("Employee ");
        write_number(i + 1);
        write_message(":\n  Username: ");
        write_message(employee.username);
        write_message("\n");
        write_message("----------------------------------------\n");

        // Send acknowledgment to the server
        int ack = 1; // Acknowledgment
        num_bytes = send(sockfd, &ack, sizeof(ack), 0);
        if (num_bytes == -1) {
            perror("send");
            close(sockfd);
            exit(1);
        }
    }

    // Close the socket
    close(sockfd);
    execvp(ManagerActionsPath, argv);
    perror("execvp failed");

    return 0; // Exit successfully
}
