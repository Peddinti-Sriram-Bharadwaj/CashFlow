#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"

struct EmployeeLogin {
    char username[20];
    char loggedin[2];
    char hashed_password[crypto_pwhash_STRBYTES]; // Store the hashed password
};

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

// Function to attempt to acquire the lock
int acquire_lock(int fd, int type, off_t start, off_t len) {
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = type; // Lock type (shared or exclusive)
    lock.l_whence = SEEK_SET;
    lock.l_start = start; // Start of lock
    lock.l_len = len; // Length of lock (0 for whole file)

    while (fcntl(fd, F_SETLK, &lock) == -1) {
        write(STDOUT_FILENO, "Lock is currently unavailable. Please wait...\n", 47);
        sleep(1); // Wait for a second before retrying
    }
    return 0; // Return success
}

int main() {
    // Initialize libsodium
    if (sodium_init() < 0) {
        return 1; // Panic! The library couldn't be initialized
    }

    char EmployeeLoginsPath[256];
    snprintf(EmployeeLoginsPath, sizeof(EmployeeLoginsPath), "%s%s", basePath, "/employee/employeelogins.txt");

    char EmployeeActionsPath[256];
    snprintf(EmployeeActionsPath, sizeof(EmployeeActionsPath), "%s%s", basePath, "/employee/employee.out");

    char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");

    int fd = open(EmployeeLoginsPath, O_RDWR); // Open for reading and writing
    if (fd < 0) {
        perror("Failed to open logins file");
        return 1; // Exit if open fails
    }

    // Attempt to acquire a write lock on the whole file
    if (acquire_lock(fd, F_WRLCK, 0, 0) == -1) {
        close(fd);
        return 1; // Exit if lock acquisition fails
    }

    struct EmployeeLogin e;
    int found = 0;
    char username[50], password[50];

    write(STDOUT_FILENO, "Welcome to the Employee dashboard\n", 34);
    write(STDOUT_FILENO, "Please login to proceed further\n", 32);
    write(STDOUT_FILENO, "Enter your username\n", 21);
    read(STDIN_FILENO, username, sizeof(username));
    remove_newline(username);

    // Search for the employee
    off_t user_position = -1; // To track the position of the found user
    while (read(fd, &e, sizeof(e)) > 0) {
        if (strcmp(e.username, username) == 0) {
            found = 1;
            user_position = lseek(fd, -sizeof(e), SEEK_CUR); // Record position if found
            break; // Exit the loop if employee is found
        }
    }

    // If employee not found, unlock the whole file and exit
    if (!found) {
        write(STDOUT_FILENO, "User doesn't exist\n", 19);
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        return 0; // Exit if user not found
    }

    // Check if the employee is already logged in
    if (strcmp(e.loggedin, "y") == 0) {
        write(STDOUT_FILENO, "User already logged in another session\n", 40);

        // Unlock the whole file before exiting
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);

        // Execute the exit path
        execvp(ExitPath, NULL);
        perror("Failed to execute exit path");
        return 1; // Exit if exec fails
    }

    // Prompt for the password
    write(STDOUT_FILENO, "Enter your password\n", 20);
    read(STDIN_FILENO, password, sizeof(password));
    remove_newline(password);

    // Verify the password
    if (crypto_pwhash_str_verify(e.hashed_password, password, strlen(password)) != 0) {
        write(STDOUT_FILENO, "Invalid password\n", 17);
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        return 0; // Exit if password verification fails
    } else {
        write(STDOUT_FILENO, "Login successful\n", 17);
        e.loggedin[0] = 'y'; // Update logged-in status
        e.loggedin[1] = '\0';

        // Move the file pointer to the position of the user
        if (lseek(fd, user_position, SEEK_SET) == (off_t)-1) {
            perror("Failed to seek to user position");
            struct flock unlock;
            memset(&unlock, 0, sizeof(unlock));
            unlock.l_type = F_UNLCK; // Unlock the whole file
            fcntl(fd, F_SETLK, &unlock);
            close(fd);
            return 1; // Exit if seeking fails
        }

        // Write the updated employee login struct
        if (write(fd, &e, sizeof(e)) != sizeof(e)) {
            perror("Failed to write updated login status");
            struct flock unlock;
            memset(&unlock, 0, sizeof(unlock));
            unlock.l_type = F_UNLCK; // Unlock the whole file
            fcntl(fd, F_SETLK, &unlock);
            close(fd);
            return 1; // Exit if write fails
        }

        write(STDOUT_FILENO, "Updated login status for user: ", 31);
        write(STDOUT_FILENO, e.username, strlen(e.username));
        write(STDOUT_FILENO, "\n", 1);

        // Unlock the whole file before executing employee actions
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd); // Close the file descriptor before executing employee actions

        // Execute the employee actions
        char *args[] = {username, NULL};
        execvp(EmployeeActionsPath, args);
        
        // If execvp fails
        perror("Failed to execute employee actions");
        return 1; // Exit if execvp fails
    }

    // This line may not be reached if execvp is successful
    struct flock unlock;
    memset(&unlock, 0, sizeof(unlock));
    unlock.l_type = F_UNLCK; // Unlock the whole file
    fcntl(fd, F_SETLK, &unlock);
    close(fd);
    return 0;
}
