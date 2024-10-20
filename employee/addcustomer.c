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

struct CustomerLogin {
    char username[20];
    char loggedin[2];
    char hashed_password[crypto_pwhash_STRBYTES]; // Store the hashed password
};

struct Customer {
    char username[20];
    int balance;
};

struct Operation {
    char operation[20];
    struct Customer customer;
};

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

int username_exists(int fd, const char *username) {
    struct CustomerLogin temp;

    // Read existing usernames from the file
    while (read(fd, &temp, sizeof(temp)) > 0) {
        if (strcmp(temp.username, username) == 0) {
            return 1; // Username exists
        }
    }
    return 0; // Username does not exist
}

int main(int argc, char *argv[]) {
    // Initialize libsodium
    if (sodium_init() < 0) {
        return 1; // Panic! The library couldn't be initialized
    }

    char EmployeeActionsPath[256];
    snprintf(EmployeeActionsPath, sizeof(EmployeeActionsPath), "%s%s", basePath, "/employee/employee.out");

    char CustomerLoginsPath[256];
    snprintf(CustomerLoginsPath, sizeof(CustomerLoginsPath), "%s%s", basePath, "/customer/customerlogins.txt");

    // Open the file for reading and writing with a lock
    int fd = open(CustomerLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("Failed to open customer login file");
        return 1;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;    // Exclusive lock
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;           // Lock the whole file

    // Attempt to acquire the write lock
    printf("Waiting to acquire the lock for writing...\n");
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(fd);
        return 1; // Lock acquisition failed
    }

    struct CustomerLogin e;
    char username[20], password[20];

    printf("Welcome to cashflow, dear customer\n");
    printf("Please create a new account\n");
    printf("Enter your username\n");
    fgets(username, sizeof(username), stdin);
    remove_newline(username);

    // Check if the username already exists
    if (username_exists(fd, username)) {
        printf("Username already exists! Please try a different username.\n");
        lock.l_type = F_UNLCK; // Unlock
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        execvp(EmployeeActionsPath, argv); // Return to employee actions page
        return 1;
    }

    printf("Enter your password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Hash the password
    if (crypto_pwhash_str(e.hashed_password, password, strlen(password), crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Password hashing failed\n");
        lock.l_type = F_UNLCK; // Unlock
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        return 1; // Error occurred
    }

    // Store the username and hashed password in the file
    strncpy(e.username, username, sizeof(e.username) - 1); // Ensure null-termination
    strncpy(e.loggedin, "n", sizeof(e.loggedin) - 1);
    
    if (write(fd, &e, sizeof(e)) == -1) {
        perror("Failed to write customer login data");
        lock.l_type = F_UNLCK; // Unlock
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        return 1; // Error occurred
    }

    printf("Account created successfully\n");
    close(fd);

    lock.l_type = F_UNLCK; // Unlock the file
    fcntl(fd, F_SETLK, &lock);

    struct Customer c;
    struct Operation o;
    strncpy(c.username, username, sizeof(c.username) - 1);
    c.balance = 0;
    strncpy(o.operation, "addcustomer", sizeof(o.operation) - 1);
    o.customer = c;

    // Socket programming to send customer data to server
    int sockfd;
    struct sockaddr_un server_addr;

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

    // Send the operation to the server
    if (send(sockfd, &o, sizeof(o), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    printf("Customer data sent to server\n");

    // Close the socket
    close(sockfd);

    // Execute the employee actions program
    execvp(EmployeeActionsPath, argv);

    return 0;
}
