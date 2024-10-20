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
        char username[20]; // For feedback submission
    } data;
};

void safe_write(int fd, const char *str) {
    write(fd, str, strlen(str));
}

void safe_read(int fd, char *buffer, size_t size) {
    ssize_t num_bytes = read(fd, buffer, size - 1);
    if (num_bytes == -1) {
        perror("read");
        exit(1);
    }
    buffer[num_bytes] = '\0'; // Null-terminate
}

int main(int argc, char *argv[]) {
    char CustomerActionsPath[256];
    snprintf(CustomerActionsPath, sizeof(CustomerActionsPath), "%s%s", basePath, "/customer/customer.out");

    char *username = argv[0]; // Use the first argument as the username
    safe_write(STDOUT_FILENO, "This is the username: ");
    safe_write(STDOUT_FILENO, username);
    safe_write(STDOUT_FILENO, "\n");

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

    // Prepare the operation for feedback submission
    strcpy(operation.operation, "feedbackSubmission");
    
    // Copy username into operation data
    strncpy(operation.data.username, username, sizeof(operation.data.username) - 1);
    operation.data.username[sizeof(operation.data.username) - 1] = '\0'; // Null-terminate

    // Send feedback request to server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Receive the "feedback" prompt message from the server
    char server_message[BUFFER_SIZE];
    safe_read(sockfd, server_message, sizeof(server_message));

    // Check if server requests feedback
    if (strcmp(server_message, "feedback") == 0) {
        // Prompt the user for feedback
        char feedback[101]; // Buffer for feedback (100 chars + null terminator)
        safe_write(STDOUT_FILENO, "Enter your feedback (max 100 characters): ");
        
        // Read feedback from stdin
        ssize_t feedback_len = read(STDIN_FILENO, feedback, sizeof(feedback) - 1);
        if (feedback_len == -1) {
            perror("read");
            close(sockfd);
            exit(1);
        }
        feedback[feedback_len] = '\0'; // Null-terminate

        // Remove newline character if present
        feedback[strcspn(feedback, "\n")] = '\0';

        // Send the feedback to the server
        num_bytes = send(sockfd, feedback, sizeof(feedback), 0);
        if (num_bytes == -1) {
            perror("send");
            close(sockfd);
            exit(1);
        }

        // Receive the result from the server (1 for success, -1 for failure)
        int result;
        num_bytes = recv(sockfd, &result, sizeof(result), 0);
        if (num_bytes == -1) {
            perror("recv");
            close(sockfd);
            exit(1);
        }

        // Print appropriate message based on server response
        if (result == 1) {
            safe_write(STDOUT_FILENO, "Feedback submitted successfully for user ");
            safe_write(STDOUT_FILENO, operation.data.username);
            safe_write(STDOUT_FILENO, ".\n");
        } else if (result == -1) {
            safe_write(STDOUT_FILENO, "Feedback submission failed for user ");
            safe_write(STDOUT_FILENO, operation.data.username);
            safe_write(STDOUT_FILENO, ".\n");
        } else {
            safe_write(STDOUT_FILENO, "Unexpected response from server.\n");
        }
    } else {
        safe_write(STDOUT_FILENO, "Unexpected message from server: ");
        safe_write(STDOUT_FILENO, server_message);
        safe_write(STDOUT_FILENO, "\n");
    }

    // Close the socket
    close(sockfd);

    // Execute the next customer action
    execvp(CustomerActionsPath, argv);
    perror("execvp failed");

    return 0; // Exit successfully
}
