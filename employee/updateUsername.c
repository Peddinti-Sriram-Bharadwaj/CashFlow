#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../global.c"
#include <sodium.h> // Required for crypto_pwhash_STRBYTES

#define SOCKET_PATH "/tmp/server"
#define BUFFER_SIZE 256

// Structure for the operation the client sends
struct Operation {
    char operation[20]; // Operation type (e.g., "updateUsername")
    char username[20];  // Current username of the customer
};


// Structure for customer login details
struct CustomerLogin {
    char username[20];                         // Username of the customer
    char loggedin[2];                          // '1' if logged in, '0' otherwise
    char hashed_password[crypto_pwhash_STRBYTES];  // Hashed password
};

int update_customer_login(const char *current_username, const char *new_username) {
    char CustomerLoginsPath[256];
    snprintf(CustomerLoginsPath, sizeof(CustomerLoginsPath), "%s%s", basePath, "/customer/customerlogins.txt");

    FILE *customer_file = fopen(CustomerLoginsPath, "rb+");
    if (customer_file == NULL) {
        perror("fopen customers.txt");
        return -1;
    }

    struct CustomerLogin customer;
    int user_found = 0;

    // Find the current user in customers.txt
    while (fread(&customer, sizeof(struct CustomerLogin), 1, customer_file) == 1) {
        if (strcmp(customer.username, current_username) == 0) {
            user_found = 1;
            break;
        }
    }

    if (!user_found) {
        fclose(customer_file);
        return -1;  // User not found
    }

    // Update the username in customers.txt
    fseek(customer_file, -sizeof(struct CustomerLogin), SEEK_CUR);  // Move back to the current record
    strncpy(customer.username, new_username, sizeof(customer.username) - 1);
    customer.username[sizeof(customer.username) - 1] = '\0';  // Null-terminate
    fwrite(&customer, sizeof(struct CustomerLogin), 1, customer_file);
    fflush(customer_file);  // Ensure changes are written to the file

    fclose(customer_file);
    return 0;  // Success
}

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

    // Ask the user for the customer whose username they want to update
    char customer_username[20];
    printf("Enter the username of the customer whose username you want to update: ");
    scanf("%19s", customer_username);  // Take input for the current customer username

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

    // Prepare the operation for updating the username
    strcpy(operation.operation, "updateUsername");
    strncpy(operation.username, customer_username, sizeof(operation.username) - 1);
    operation.username[sizeof(operation.username) - 1] = '\0';  // Null-terminate

    // Send the operation request to the server
    ssize_t num_bytes = send(sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Receive the server's message asking for the new username
    char server_message[BUFFER_SIZE];
    num_bytes = recv(sockfd, server_message, sizeof(server_message), 0);
    if (num_bytes == -1) {
        perror("recv server_message");
        close(sockfd);
        exit(1);
    }
    server_message[num_bytes] = '\0';  // Null-terminate the message
    if (strcmp(server_message, "newname") == 0) {
        // Server is asking for the new username
        char new_username[20];
        printf("Enter the new username for %s: ", customer_username);
        scanf("%19s", new_username);  // Take input for the new username

        // Send the new username to the server
        num_bytes = send(sockfd, new_username, sizeof(new_username), 0);
        if (num_bytes == -1) {
            perror("send new_username");
            close(sockfd);
            exit(1);
        }

        // Receive the server's response (success or failure)
        int response;
        num_bytes = recv(sockfd, &response, sizeof(response), 0);
        if (num_bytes == -1) {
            perror("recv response");
            close(sockfd);
            exit(1);
        }

        if (response == 1) {
            printf("Username updated successfully on the server.\n");

            // Now, update the "customers.txt" file to reflect the new username
            if (update_customer_login(customer_username, new_username) == 0) {
                printf("Username updated successfully in customers.txt.\n");
            } else {
                printf("Failed to update username in customers.txt.\n");
            }
        } else if (response == -1) {
            printf("Failed to update username: New username already exists.\n");
        }
    } else {
        printf("Unexpected message from server: %s\n", server_message);
    }

    // Close the socket
    close(sockfd);

    execvp(EmployeeActionsPath, argv);  // Return to the employee actions menu

    return 0; // Exit successfully
}
