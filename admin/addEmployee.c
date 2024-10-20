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

pthread_rwlock_t employees_lock = PTHREAD_RWLOCK_INITIALIZER;

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

int username_exists(const char *filepath, const char *username) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 0;
    }

    struct EmployeeLogin temp;
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
    char EmployeeLoginsPath[256];
    snprintf(EmployeeLoginsPath, sizeof(EmployeeLoginsPath), "%s%s", basePath, "/employee/employeelogins.txt");

    char AdminActionsPath[256];
    snprintf(AdminActionsPath, sizeof(AdminActionsPath), "%s%s", basePath, "/admin/admin.out");

    // Initialize libsodium
    if (sodium_init() < 0) {
        printf("libsodium initialization failed\n");
        return 1;
    }

    printf("Please enter the name of the Employee to be added\n");
    printf("Enter the username\n");
    char username[20], password[20];
    fgets(username, sizeof(username), stdin);
    remove_newline(username);

    // Lock for reading to check username existence
    pthread_rwlock_wrlock(&employees_lock); // Lock for writing to ensure exclusivity
    
    // Check if username already exists
    if (username_exists(EmployeeLoginsPath, username)) {
        pthread_rwlock_unlock(&employees_lock); // Unlock after checking
        printf("Username already exists! Please try a different username.\n");
        
        // Execute admin actions path and pass argv
        execvp(AdminActionsPath, argv); // Return to admin actions page
        perror("execvp failed");
        return 1;
    }

    // Proceed to add the employee only if the username does not exist
    printf("Ask the employee to create a password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    struct EmployeeLogin e;
    strcpy(e.username, username);

    // Hash the password
    if (crypto_pwhash_str(e.hashed_password, password, strlen(password), 
                           crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                           crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Error hashing the password\n");
        pthread_rwlock_unlock(&employees_lock); // Unlock before returning
        return 1;
    }
    
    strncpy(e.loggedin, "n", sizeof(e.loggedin) - 1);

    // Prepare the operation structure
    struct Operation op;
    strncpy(op.operation, "addemployee", sizeof(op.operation) - 1);
    op.employee = e;

    // Write to the employee login file (using system call)
    int fd = open(EmployeeLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        printf("Failed to open the file\n");
        pthread_rwlock_unlock(&employees_lock);
        return 1;
    }

    // Lock for writing to ensure safe file writing
    pthread_rwlock_wrlock(&employees_lock);
    // Write the employee data to the file
    ssize_t bytes_written = write(fd, &e, sizeof(e));
    if (bytes_written != sizeof(e)) {
        perror("Failed to write employee data");
        pthread_rwlock_unlock(&employees_lock); // Unlock before returning
        close(fd);
        return 1;
    }

    close(fd);
    printf("Employee added successfully\n");

    pthread_rwlock_unlock(&employees_lock); // Unlock after writing employee data

    // Socket programming to send employee data to server using stream sockets
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

    close(sockfd);

    execvp(AdminActionsPath, argv);

    return 0;
}
