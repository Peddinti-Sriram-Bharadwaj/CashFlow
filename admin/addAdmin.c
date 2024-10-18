#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

#define SOCKET_PATH "/tmp/server"
#define BUFFER_SIZE 256

struct AdminLogin {
    char username[20];
    char loggedin[2];
    char hashed_password[crypto_pwhash_STRBYTES]; // Store the hashed password
};

struct Operation {
    char operation[20];
    struct AdminLogin admin;
};

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

int main() {
    // Initialize libsodium
    if (sodium_init() < 0) {
        printf("libsodium initialization failed\n");
        return 1;
    }

    printf("Please enter the name of the customer to be added\n");
    printf("Enter the username\n");
    char username[20], password[20];
    fgets(username, sizeof(username), stdin);
    remove_newline(username);

    printf("Ask the customer to create a password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Hash the password
    struct AdminLogin a;
    if (crypto_pwhash_str(a.hashed_password, password, strlen(password), 
                           crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                           crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Password hashing failed\n");
        return 1;
    }

    // Store the username and hashed password in the struct
    strncpy(a.username, username, sizeof(a.username) - 1); // Ensure null-termination
    strncpy(a.loggedin, "n", sizeof(a.loggedin) - 1);

    // Prepare the operation structure
    struct Operation op;
    strncpy(op.operation, "addadmin", sizeof(op.operation) - 1);
    op.admin = a;

    // Write to the admin login file
    char AdminLoginsPath[256];
    snprintf(AdminLoginsPath, sizeof(AdminLoginsPath), "%s%s", basePath, "/admin/adminlogins.txt");

    int fd = open(AdminLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        printf("Failed to open the file\n");
        return 1;
    }

    // Write the struct with username and hashed password to the file
    write(fd, &a, sizeof(a));
    close(fd);

    printf("Admin account created and data written to file\n");

    // Socket programming to send admin data to server
    int sockfd;
    struct sockaddr_un server_addr;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;

    // Set up socket path
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    // Send the operation to the server
    if (send(sockfd, &op, sizeof(op), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    printf("Admin data sent to server\n");

    // Close the socket
    close(sockfd);

    return 0;
}
