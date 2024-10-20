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
        char username[20]; // For loan application (username)
    } data;
};

struct LoanApplication {
    char username[20];
    int amount;
    char assigned_employee[20]; // Initially set to "None"
};

void write_message(int fd, const char *message) {
    write(fd, message, strlen(message));
}

void write_int(int fd, int value) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%d\n", value);
    write(fd, buffer, strlen(buffer));
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

    // Prepare the operation to list pending applications
    strcpy(operation.operation, "pending");

    // Send the operation to the server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Receive the number of pending applications
    int pending_count;
    num_bytes = read(sockfd, &pending_count, sizeof(pending_count));
    if (num_bytes == -1) {
        perror("read");
        close(sockfd);
        exit(1);
    }

    write_message(STDOUT_FILENO, "Number of pending loan applications: ");
    write_int(STDOUT_FILENO, pending_count);

    // Step 4: Receive each pending loan application and display it
    for (int i = 0; i < pending_count; i++) {
        struct LoanApplication loan_application;
        num_bytes = read(sockfd, &loan_application, sizeof(struct LoanApplication));
        if (num_bytes == -1) {
            perror("read");
            close(sockfd);
            exit(1);
        }

        // Display the loan application
        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), "Application %d:\n  Username: %s\n  Amount: %d\n  Assigned Employee: %s\n", 
                 i + 1, loan_application.username, loan_application.amount, loan_application.assigned_employee);
        write(STDOUT_FILENO, buffer, strlen(buffer));

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
