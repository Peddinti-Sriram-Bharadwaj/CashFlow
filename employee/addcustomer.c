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

struct Customer{
    char username[20];
    int balance;
};

struct Operation{
    char operation[20];
    struct Customer customer;
};

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
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

    int fd = open(CustomerLoginsPath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    struct CustomerLogin e;
    char username[20], password[20];

    printf("Welcome to cashflow, dear customer\n");
    printf("Please create a new account\n");
    printf("Enter your username\n");
    fgets(username, sizeof(username), stdin);
    remove_newline(username);

    printf("Enter your password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Hash the password
    if (crypto_pwhash_str(e.hashed_password, password, strlen(password), crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Password hashing failed\n");
        close(fd);
        return 1;
    }

    // Store the username and hashed password in the file
    strncpy(e.username, username, sizeof(e.username) - 1); // Ensure null-termination
    strncpy(e.loggedin, "n", sizeof(e.loggedin) - 1);
    write(fd, &e, sizeof(e));

    printf("Account created successfully\n");
    close(fd);

    struct Customer c;
    struct Operation o;
    strncpy(c.username, username, sizeof(c.username) - 1);
    strncpy(o.operation, "addcustomer", sizeof(o.operation) - 1);
    o.customer = c;
    c.balance = 0;

    int sockfd;
    struct sockaddr_un server_addr;

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    //datagram client
    if (sendto(sockfd, &o, sizeof(o), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("sendto");
        close(sockfd);
        exit(1);
    }

    close(sockfd);

    execvp(EmployeeActionsPath, argv);

    return 0;
}
