#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"
#include <pthread.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

#define SOCKET_PATH "/tmp/server"
#define BUFFER_SIZE 256

pthread_rwlock_t customers_lock = PTHREAD_RWLOCK_INITIALIZER;

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

int username_exists(const char *filepath, const char *username) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 0;
    }

    struct CustomerLogin temp;
    while (read(fd, &temp, sizeof(temp)) > 0) {
        if (strcmp(temp.username, username) == 0) {
            close(fd);
            return 1; // Username exists
        }
    }

    close(fd);
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

    // Lock customers for writing
    pthread_rwlock_wrlock(&customers_lock);

    struct CustomerLogin e;
    char username[20], password[20];

    printf("Welcome to cashflow, dear customer\n");
    printf("Please create a new account\n");
    printf("Enter your username\n");
    fgets(username, sizeof(username), stdin);
    remove_newline(username);

    // Check if username already exists
    if (username_exists(CustomerLoginsPath, username)) {
        printf("Username already exists! Please try a different username.\n");
        pthread_rwlock_unlock(&customers_lock);
        execvp(EmployeeActionsPath, argv); // Return to employee actions page
        return 1;
    }

    printf("Enter your password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Hash the password
    if (crypto_pwhash_str(e.hashed_password, password, strlen(password), crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Password hashing failed\n");
        pthread_rwlock_unlock(&customers_lock);
        return 1;
    }

    // Open the file to write customer login data (using system call)
    int fd = open(CustomerLoginsPath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        printf("Failed to open customer login file\n");
        pthread_rwlock_unlock(&customers_lock);
        return 1;
    }

    // Store the username and hashed password in the file (using system call)
    strncpy(e.username, username, sizeof(e.username) - 1); // Ensure null-termination
    strncpy(e.loggedin, "n", sizeof(e.loggedin) - 1);
    write(fd, &e, sizeof(e));

    printf("Account created successfully\n");
    close(fd);

    pthread_rwlock_unlock(&customers_lock); // Unlock after writing customer data

    struct Customer c;
    struct Operation o;
    strncpy(c.username, username, sizeof(c.username) - 1);
    strncpy(o.operation, "addcustomer", sizeof(o.operation) - 1);
    o.customer = c;
    c.balance = 0;

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
