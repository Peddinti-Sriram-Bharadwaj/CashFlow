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

pthread_rwlock_t managers_lock = PTHREAD_RWLOCK_INITIALIZER;

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

int username_exists(const char *filepath, const char *username) {
    // Acquire read lock
    pthread_rwlock_rdlock(&managers_lock);

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("open");
        pthread_rwlock_unlock(&managers_lock);
        return 0;
    }

    struct ManagerLogin temp;
    while (read(fd, &temp, sizeof(temp)) > 0) {
        if (strcmp(temp.username, username) == 0) {
            close(fd);
            pthread_rwlock_unlock(&managers_lock);
            return 1; // Username exists
        }
    }

    close(fd);
    pthread_rwlock_unlock(&managers_lock); // Unlock after checking
    return 0; // Username does not exist
}

int main(int argc, char *argv[]) {
    char ManagerLoginsPath[256];
    snprintf(ManagerLoginsPath, sizeof(ManagerLoginsPath), "%s%s", basePath, "/Manager/managerlogins.txt");

    char AdminActionsPath[256];
    snprintf(AdminActionsPath, sizeof(AdminActionsPath), "%s%s", basePath, "/admin/admin.out");

    // Initialize libsodium
    if (sodium_init() < 0) {
        printf("libsodium initialization failed\n");
        return 1;
    }

    printf("Please enter the name of the Manager to be added\n");
    printf("Enter the username\n");
    char username[20], password[20];
    fgets(username, sizeof(username), stdin);
    remove_newline(username);

    // Check if username already exists
    if (username_exists(ManagerLoginsPath, username)) {
        printf("Username already exists! Redirecting to admin actions.\n");
        execvp(AdminActionsPath, argv); // Return to admin actions page
        perror("execvp failed"); // Only reached if execvp fails
        return 1;
    }

    printf("Ask the Manager to create a password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Hash the password
    struct ManagerLogin e;
    if (crypto_pwhash_str(e.hashed_password, password, strlen(password), 
                           crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                           crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Password hashing failed\n");
        return 1;
    }

    // Store the username and hashed password
    strncpy(e.username, username, sizeof(e.username) - 1); // Ensure null-termination
    strncpy(e.loggedin, "n", sizeof(e.loggedin) - 1);

    // Prepare the operation structure
    struct Operation op;
    strncpy(op.operation, "addmanager", sizeof(op.operation) - 1);
    op.manager = e;

    // Lock managers for writing
    pthread_rwlock_wrlock(&managers_lock);

    // Write to the manager login file (using system call)
    int fd = open(ManagerLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        printf("Failed to open the file\n");
        pthread_rwlock_unlock(&managers_lock);
        return 1;
    }

    if (write(fd, &e, sizeof(e)) == -1) {
        perror("Failed to write to the file");
        close(fd);
        pthread_rwlock_unlock(&managers_lock);
        return 1;
    }

    printf("Manager account created successfully\n");
    close(fd);

    pthread_rwlock_unlock(&managers_lock); // Unlock after writing manager data

    // Socket programming to send manager data to server using stream sockets
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

    printf("Manager data sent to server\n");

    // Close the socket
    close(sockfd);
    execvp(AdminActionsPath, argv); // Ensure redirection to admin actions after completion

    return 0;
}
