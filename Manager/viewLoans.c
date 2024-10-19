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

struct LoanApplication {
    char username[20];
    int amount;
    char assigned_employee[20]; // Initially set to "None"
};

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
    // You can add additional data to the operation if needed
    // For example, operation.data.username could be set if you wanted to filter by a specific user

    // Send the operation to the server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Receive the number of pending applications
    int pending_count;
    num_bytes = recv(sockfd, &pending_count, sizeof(pending_count), 0);
    if (num_bytes == -1) {
        perror("recv");
        close(sockfd);
        exit(1);
    }

    printf("Number of pending loan applications: %d\n", pending_count);

    // Step 4: Receive each pending loan application and display it
    for (int i = 0; i < pending_count; i++) {
        struct LoanApplication loan_application;
        num_bytes = recv(sockfd, &loan_application, sizeof(struct LoanApplication), 0);
        if (num_bytes == -1) {
            perror("recv");
            close(sockfd);
            exit(1);
        }

        // Display the loan application
        printf("Application %d:\n", i + 1);
        printf("  Username: %s\n", loan_application.username);
        printf("  Amount: %d\n", loan_application.amount);
        printf("  Assigned Employee: %s\n", loan_application.assigned_employee);

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
