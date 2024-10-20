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

struct ManagerLogin {
    char username[20];
    char loggedin[2];
    char hashed_password[crypto_pwhash_STRBYTES]; // Store the hashed password
};

struct Operation {
    char operation[20];
    struct ManagerLogin manager;
};

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

int username_exists(int fd, const char *username) {
    struct ManagerLogin temp;
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

void add_manager(int fd, struct ManagerLogin *m) {
    // Write the manager data to the file
    ssize_t bytes_written = write(fd, m, sizeof(*m));
    if (bytes_written != sizeof(*m)) {
        perror("Failed to write manager data");
    } else {
        write(STDOUT_FILENO, "Manager added successfully\n", 27);
    }
}

int main(int argc, char *argv[]) {
    char ManagerLoginsPath[256];
    sprintf(ManagerLoginsPath, "%s%s", basePath, "/Manager/managerlogins.txt");

    char AdminActionsPath[256];
    sprintf(AdminActionsPath, "%s%s", basePath, "/admin/admin.out");

    // Initialize libsodium
    if (sodium_init() < 0) {
        write(STDOUT_FILENO, "libsodium initialization failed\n", 32);
        execvp(AdminActionsPath, argv); // Redirect to admin actions on failure
        perror("execvp failed"); // Only reached if execvp fails
        return 1;
    }

    // Open the file for reading and writing with a lock
    int fd = open(ManagerLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("Failed to open the file");
        execvp(AdminActionsPath, argv); // Redirect to admin actions on failure
        return 1;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;    // Exclusive lock
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;           // Lock the whole file

    // Attempt to acquire the write lock
    write(STDOUT_FILENO, "Waiting to acquire the lock for writing...\n", 43);
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(fd);
        execvp(AdminActionsPath, argv); // Redirect to admin actions on failure
        return 1;
    }

    // Ask for username
    char username[20], password[20];
    write(STDOUT_FILENO, "Please enter the name of the Manager to be added\n", 50);
    write(STDOUT_FILENO, "Enter the username\n", 19);
    read(STDIN_FILENO, username, sizeof(username));
    remove_newline(username);

    // Check if the username exists
    if (username_exists(fd, username)) {
        write(STDOUT_FILENO, "Username already exists! Redirecting to admin actions...\n", 58);
        lock.l_type = F_UNLCK; // Unlock
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        execvp(AdminActionsPath, argv); // Redirect to admin actions
        return 1;
    }

    // Ask the manager to create a password
    write(STDOUT_FILENO, "Ask the Manager to create a password\n", 37);
    read(STDIN_FILENO, password, sizeof(password));
    remove_newline(password);

    struct ManagerLogin m;
    strncpy(m.username, username, sizeof(m.username) - 1);
    m.username[sizeof(m.username) - 1] = '\0'; // Ensure null-termination

    // Hash the password
    if (crypto_pwhash_str(m.hashed_password, password, strlen(password), 
                           crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                           crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        write(STDOUT_FILENO, "Error hashing the password\n", 27);
        lock.l_type = F_UNLCK; // Unlock in case of error
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        execvp(AdminActionsPath, argv); // Redirect to admin actions on failure
        return 1;
    }

    strncpy(m.loggedin, "n", sizeof(m.loggedin) - 1);
    m.loggedin[sizeof(m.loggedin) - 1] = '\0'; // Ensure null-termination

    // Add the manager
    add_manager(fd, &m);

    // Unlock the file
    lock.l_type = F_UNLCK; // Unlock
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    // Socket programming to send manager data to server using stream sockets
    int sockfd;
    struct sockaddr_un server_addr;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        execvp(AdminActionsPath, argv); // Redirect to admin actions on failure
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
        execvp(AdminActionsPath, argv); // Redirect to admin actions on failure
        return 1;
    }

    // Prepare the operation structure
    struct Operation op;
    strncpy(op.operation, "addmanager", sizeof(op.operation) - 1);
    op.manager = m;

    // Send the operation to the server
    if (send(sockfd, &op, sizeof(op), 0) == -1) {
        perror("send");
        close(sockfd);
        execvp(AdminActionsPath, argv); // Redirect to admin actions on failure
        return 1;
    }

    close(sockfd);

    execvp(AdminActionsPath, argv); // Redirect to admin actions after completion
    perror("execvp failed"); // Only reached if execvp fails
    return 1;
}
