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

struct EmployeeLogin {
    char username[20];
    char loggedin[2];
    char hashed_password[crypto_pwhash_STRBYTES]; // Store the hashed password
};

struct Operation {
    char operation[20];
    struct EmployeeLogin employee;
};

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

int username_exists(int fd, const char *username) {
    struct EmployeeLogin temp;
    int found = 0;

    // Read existing usernames from the file
    while (read(fd, &temp, sizeof(temp)) > 0) {
        if (strcmp(temp.username, username) == 0) {
            found = 1; // Username exists
            break;
        }
    }

    return found; // Return whether the username exists
}

void add_employee(int fd, struct EmployeeLogin *e) {
    // Write the employee data to the file
    ssize_t bytes_written = write(fd, e, sizeof(*e));
    if (bytes_written != sizeof(*e)) {
        perror("Failed to write employee data");
    } else {
        printf("Employee added successfully\n");
    }
}

int main(int argc, char *argv[]) {
    char EmployeeLoginsPath[256];
    snprintf(EmployeeLoginsPath, sizeof(EmployeeLoginsPath), "%s%s", basePath, "/employee/employeelogins.txt");

    char AdminActionsPath[256];
    snprintf(AdminActionsPath, sizeof(AdminActionsPath), "%s%s", basePath, "/admin/admin.out");

    char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");

    // Initialize libsodium
    if (sodium_init() < 0) {
        printf("libsodium initialization failed\n");
        execvp(ExitPath, argv); // Go to ExitPath on error
        perror("execvp failed");
        return 1;
    }

    // Open the file for reading and writing with a lock
    int fd = open(EmployeeLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("Failed to open the file for reading and writing");
        execvp(ExitPath, argv); // Go to ExitPath on error
        return 1;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;    // Exclusive lock
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;           // Lock the whole file

    // Attempt to acquire the write lock
    printf("Waiting to acquire the lock for writing...\n"); // Print when waiting
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(fd);
        execvp(ExitPath, argv); // Go to ExitPath on error
        return 1;
    }

    // Ask for username
    char username[20], password[20];
    printf("Please enter the name of the Employee to be added\n");
    printf("Enter the username\n");
    if (read(STDIN_FILENO, username, sizeof(username)) == -1) {
        perror("read");
        lock.l_type = F_UNLCK; // Unlock
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        execvp(ExitPath, argv); // Go to ExitPath on error
        return 1;
    }
    remove_newline(username);

    // Check if the username exists
    if (username_exists(fd, username)) {
        printf("Username already exists! Redirecting to admin actions...\n");
        lock.l_type = F_UNLCK; // Unlock
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        execvp(AdminActionsPath, argv); // Redirect to admin actions
        return 1;
    }

    // Ask the employee to create a password
    printf("Ask the employee to create a password\n");
    if (read(STDIN_FILENO, password, sizeof(password)) == -1) {
        perror("read");
        lock.l_type = F_UNLCK; // Unlock
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        execvp(ExitPath, argv); // Go to ExitPath on error
        return 1;
    }
    remove_newline(password);

    struct EmployeeLogin e;
    strncpy(e.username, username, sizeof(e.username) - 1);

    // Hash the password
    if (crypto_pwhash_str(e.hashed_password, password, strlen(password), 
                           crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                           crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Error hashing the password\n");
        lock.l_type = F_UNLCK; // Unlock in case of error
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        execvp(ExitPath, argv); // Go to ExitPath on error
        return 1;
    }
    
    strncpy(e.loggedin, "n", sizeof(e.loggedin) - 1);

    // Add the employee
    add_employee(fd, &e);

    // Unlock the file
    lock.l_type = F_UNLCK; // Unlock
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    // Socket programming to send employee data to server using stream sockets
    int sockfd;
    struct sockaddr_un server_addr;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        execvp(ExitPath, argv); // Go to ExitPath on error
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    
    // Set up socket path
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        execvp(ExitPath, argv); // Go to ExitPath on error
        return 1;
    }

    // Prepare the operation structure
    struct Operation op;
    strncpy(op.operation, "addemployee", sizeof(op.operation) - 1);
    op.employee = e;

    // Send the operation to the server
    if (send(sockfd, &op, sizeof(op), 0) == -1) {
        perror("send");
        close(sockfd);
        execvp(ExitPath, argv); // Go to ExitPath on error
        return 1;
    }

    close(sockfd);

    execvp(AdminActionsPath, argv); // Return to admin actions
    perror("execvp failed"); // In case execvp fails
    return 1;
}
