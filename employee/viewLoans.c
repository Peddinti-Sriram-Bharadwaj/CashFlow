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

struct LoanApplication {
    char username[20];
    int amount;
    char assigned_employee[20]; // Initially set to "None"
};

int main(int argc, char *argv[]) {
    char EmployeeActionsPath[256];
    snprintf(EmployeeActionsPath, sizeof(EmployeeActionsPath), "%s%s", basePath, "/employee/employee.out");

    char* employee_username = argv[0];



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

    // Prepare the operation to request employee loans
    strcpy(operation.operation, "getEmployeeLoans");
    strcpy(operation.data.username, employee_username); // Set the employee username in the operation

    // Send the operation to the server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Receive the number of loans assigned to the employee
    int loan_count;
    num_bytes = recv(sockfd, &loan_count, sizeof(loan_count), 0);
    if (num_bytes == -1) {
        perror("recv");
        close(sockfd);
        exit(1);
    }

    printf("Number of loans assigned to %s: %d\n", employee_username, loan_count);

    // Step 4: Receive and display each loan's details
    for (int i = 0; i < loan_count; i++) {
        struct LoanApplication loan_application;
        num_bytes = recv(sockfd, &loan_application, sizeof(struct LoanApplication), 0);
        if (num_bytes == -1) {
            perror("recv");
            close(sockfd);
            exit(1);
        }

        // Display loan details
        printf("Loan %d:\n", i + 1);
        printf("  Username: %s\n", loan_application.username);
        printf("  Amount: %d\n", loan_application.amount);
    }

    // Close the socket
    close(sockfd);
    execvp(EmployeeActionsPath, argv);

    return 0;
}