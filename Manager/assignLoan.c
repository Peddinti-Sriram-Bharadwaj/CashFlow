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
    char ManagerActionsPath[256];
    snprintf(ManagerActionsPath, sizeof(ManagerActionsPath), "%s%s", basePath, "/manager/manager.out");

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

    // Prepare the operation to assign loan to employee
    strcpy(operation.operation, "assignLoan");

    // Send the operation to the server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Step 1: Wait for "applicant" keyword from server
    char buffer[BUFFER_SIZE];
    num_bytes = recv(sockfd, buffer, sizeof(buffer), 0);
    if (num_bytes == -1) {
        perror("recv");
        close(sockfd);
        exit(1);
    }

    if (strcmp(buffer, "applicant") == 0) {
        // Ask the user to input loan applicant username
        printf("Enter the loan applicant username: ");
        fgets(operation.data.username, sizeof(operation.data.username), stdin);
        operation.data.username[strcspn(operation.data.username, "\n")] = '\0'; // Remove newline

        // Send the loan applicant username to the server
        num_bytes = send(sockfd, &operation.data.username, sizeof(operation.data.username), 0);
        if (num_bytes == -1) {
            perror("send applicant");
            close(sockfd);
            exit(1);
        }
    }

    // Step 2: Wait for "employee" keyword from server
    num_bytes = recv(sockfd, buffer, sizeof(buffer), 0);
    if (num_bytes == -1) {
        perror("recv");
        close(sockfd);
        exit(1);
    }

    if (strcmp(buffer, "employee") == 0) {
        // Ask the user to input employee username
        printf("Enter the employee username: ");
        fgets(operation.data.username, sizeof(operation.data.username), stdin);
        operation.data.username[strcspn(operation.data.username, "\n")] = '\0'; // Remove newline

        // Send the employee username to the server
        num_bytes = send(sockfd, &operation.data.username, sizeof(operation.data.username), 0);
        if (num_bytes == -1) {
            perror("send employee");
            close(sockfd);
            exit(1);
        }
    }

    // Step 3: Receive the result of the loan assignment from the server
    char assignment_status[BUFFER_SIZE];
    num_bytes = recv(sockfd, assignment_status, sizeof(assignment_status), 0);
    if (num_bytes == -1) {
        perror("recv");
        close(sockfd);
        exit(1);
    }

    // Display the assignment result
    printf("Assignment result: %s\n", assignment_status);

    // Close the socket and exit
    close(sockfd);
    execvp(ManagerActionsPath, argv);
    perror("execvp failed");

    return 0; // Exit successfully
}
