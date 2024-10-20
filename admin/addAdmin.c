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
#include <errno.h>

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

void lock_file(int fd, int lock_type) {
    struct flock fl;
    fl.l_type = lock_type; // Set the lock type (read or write)
    fl.l_whence = SEEK_SET;
    fl.l_start = 0; // Lock the whole file
    fl.l_len = 0; // Lock all

    // Attempt to acquire the lock; if blocked, wait
    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        perror("Failed to lock the file");
        exit(1);
    }
}

void unlock_file(int fd) {
    struct flock fl;
    fl.l_type = F_UNLCK; // Unlock
    fl.l_whence = SEEK_SET;
    fl.l_start = 0; // Unlock the whole file
    fl.l_len = 0; // Unlock all

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("Failed to unlock the file");
        exit(1);
    }
}

int username_exists(const char *username, const char *file_path) {
    struct AdminLogin a;
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open the file");
        return 0; // Assume username does not exist
    }

    // Lock the file for reading
    lock_file(fd, F_RDLCK); // Acquire a read lock

    // Check if the username exists
    while (read(fd, &a, sizeof(a)) > 0) {
        if (strcmp(a.username, username) == 0) {
            unlock_file(fd); // Unlock before returning
            close(fd);
            return 1; // Username exists
        }
    }

    unlock_file(fd); // Unlock after checking
    close(fd);
    return 0; // Username does not exist
}

int main() {
    // Initialize libsodium
    if (sodium_init() < 0) {
        const char *msg = "libsodium initialization failed\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return 1;
    }

    const char *prompt1 = "Please enter the name of the customer to be added\nEnter the username\n";
    write(STDOUT_FILENO, prompt1, strlen(prompt1));
    char username[20], password[20];
    read(STDIN_FILENO, username, sizeof(username));
    remove_newline(username);

    // Prepare the admin login file path
    char AdminLoginsPath[256];
    sprintf(AdminLoginsPath, "%s%s", basePath, "/admin/adminlogins.txt");

    // Check if the username already exists in the admin logins
    if (username_exists(username, AdminLoginsPath)) {
        const char *msg = "Username already exists! Please try a different username.\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return 1; // Return or execvp to the admin actions path
    }

    const char *prompt2 = "Ask the customer to create a password\n";
    write(STDOUT_FILENO, prompt2, strlen(prompt2));
    read(STDIN_FILENO, password, sizeof(password));
    remove_newline(password);

    // Hash the password
    struct AdminLogin a;
    if (crypto_pwhash_str(a.hashed_password, password, strlen(password), 
                           crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                           crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        const char *msg = "Password hashing failed\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return 1;
    }

    // Store the username and hashed password in the struct
    strncpy(a.username, username, sizeof(a.username) - 1); // Ensure null-termination
    strncpy(a.loggedin, "n", sizeof(a.loggedin) - 1);

    // Prepare the operation structure
    struct Operation op;
    strncpy(op.operation, "addadmin", sizeof(op.operation) - 1);
    op.admin = a;

    // Open the file for writing
    int fd = open(AdminLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        const char *msg = "Failed to open the file\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return 1;
    }

    // Lock the file for writing
    lock_file(fd, F_WRLCK); // Acquire a write lock

    // Write the struct with username and hashed password to the file
    if (write(fd, &a, sizeof(a)) == -1) {
        perror("Failed to write to the file");
        unlock_file(fd); // Unlock before returning
        close(fd);
        return 1;
    }

    const char *msg = "Admin account created and data written to file\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    // Unlock the record after writing
    unlock_file(fd);
    close(fd);

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

    const char *msg2 = "Admin data sent to server\n";
    write(STDOUT_FILENO, msg2, strlen(msg2));

    // Close the socket
    close(sockfd);

    return 0;
}
