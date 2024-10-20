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
    char operation[20]; // Operation type (viewFeedback)
    char username[20];  // Username of the customer
};

struct Feedback {
    char username[20];
    char feedback[101]; // Feedback field with max length of 100 chars + null terminator
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

int main(int argc, char *argv[]) {
    if (argc < 1) {
        write_string(STDERR_FILENO, "Usage: ");
        write_string(STDERR_FILENO, argv[0]);
        write_string(STDERR_FILENO, " <employee_username>\n");
        exit(1);
    }

    char *employee_username = argv[0]; // Use argv[0] as the employee username
    write_string(STDOUT_FILENO, "Employee: ");
    write_string(STDOUT_FILENO, employee_username);
    write_string(STDOUT_FILENO, "\n");

    char ManagerActionsPath[BUFFER_SIZE];
    snprintf(ManagerActionsPath, sizeof(ManagerActionsPath), "%s%s", basePath, "/Manager/manager.out");

    // Ask the user for the username to view feedback
    char customer_username[20];
    write_string(STDOUT_FILENO, "Enter the username of the customer whose feedback you want to view: ");
    read_string(STDIN_FILENO, customer_username, sizeof(customer_username));

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

    // Prepare the operation for viewing feedback
    strcpy(operation.operation, "viewFeedback");
    strncpy(operation.username, customer_username, sizeof(operation.username) - 1);
    operation.username[sizeof(operation.username) - 1] = '\0'; // Null-terminate

    // Send the operation request to the server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Receive feedback response from the server
    struct Feedback feedback;
    num_bytes = read(sockfd, &feedback, sizeof(feedback));
    if (num_bytes == -1) {
        perror("Failed to read feedback");
    } else {
        write_string(STDOUT_FILENO, "Feedback:\n");
        write_string(STDOUT_FILENO, feedback.feedback);
        write_string(STDOUT_FILENO, "\n");
    }

    // Close the socket
    close(sockfd);
    execvp(ManagerActionsPath, argv); // Execute the manager actions
    return 0; // Exit successfully
}
