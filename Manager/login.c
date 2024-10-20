#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include <termios.h> // Include termios for terminal control
#include "../global.c"

struct AdminLogin {
    char username[20];
    char loggedin[2];
    char hashed_password[crypto_pwhash_STRBYTES]; // Store the hashed password
};

// Function to remove the newline character from strings
void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

// Function to acquire a lock
int acquire_lock(int fd, int type) {
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = type; // Lock type (shared or exclusive)
    lock.l_whence = SEEK_SET;
    lock.l_start = 0; // Start of lock
    lock.l_len = 0; // Length of lock (0 for whole file)

    while (fcntl(fd, F_SETLK, &lock) == -1) {
        write(STDOUT_FILENO, "Lock is currently unavailable. Please wait...\n", 47);
        sleep(1); // Wait for a second before retrying
    }
    return 0; // Return success
}

// Function to unlock the file
void unlock_file(int fd) {
    struct flock unlock;
    memset(&unlock, 0, sizeof(unlock));
    unlock.l_type = F_UNLCK; // Unlock the whole file
    fcntl(fd, F_SETLK, &unlock);
}

// Function to read password with echoing asterisks
void read_password(char *password, size_t size) {
    struct termios oldt, newt;

    // Get the current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt; // Copy old settings to new settings
    newt.c_lflag &= ~(ECHO); // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply the new settings

    // Read password
    for (size_t i = 0; i < size - 1; i++) { // Leave space for null terminator
        char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1) {
            if (ch == '\n') {
                break; // Stop on newline
            }
            password[i] = ch; // Store the character
            write(STDOUT_FILENO, "*", 1); // Print asterisk
        }
    }
    password[size - 1] = '\0'; // Null terminate the string

    // Restore old terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore original settings
}

int main() {
    // Initialize libsodium
    if (sodium_init() < 0) {
        return 1; // Panic! The library couldn't be initialized
    }

    char ManagerLoginsPath[256];
    snprintf(ManagerLoginsPath, sizeof(ManagerLoginsPath), "%s%s", basePath, "/manager/managerlogins.txt");

    char ManagerActionsPath[256];
    snprintf(ManagerActionsPath, sizeof(ManagerActionsPath), "%s%s", basePath, "/manager/manager.out");

    char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");

    int fd = open(ManagerLoginsPath, O_RDWR); // Open for reading and writing
    if (fd < 0) {
        perror("Failed to open manager logins file");
        return 1; // Exit if open fails
    }

    // Attempt to acquire a write lock on the whole file
    if (acquire_lock(fd, F_WRLCK) == -1) {
        close(fd);
        return 1; // Exit if lock acquisition fails
    }

    struct AdminLogin a;
    int found = 0;
    char username[50], password[50];

    // Ask for username input
    write(STDOUT_FILENO, "========================================\n", 41);
    write(STDOUT_FILENO, "Welcome to the admin dashboard\n", 31);
    write(STDOUT_FILENO, "Please login to proceed further\n", 32);
    write(STDOUT_FILENO, "========================================\n", 41);
    write(STDOUT_FILENO, "Enter username: ", 16);
    read(STDIN_FILENO, username, sizeof(username));
    remove_newline(username);

    // Search for the entered username and verify the password
    while (read(fd, &a, sizeof(a)) > 0) {
        if (strcmp(a.username, username) == 0) {
            found = 1;
            break; // Exit the loop if user is found
        }
    }

    // If user not found, unlock and execute exit path
    if (!found) {
        write(STDOUT_FILENO, "User doesn't exist\n", 19);
        unlock_file(fd);
        close(fd);
        execvp(ExitPath, NULL); // Execute exit path
        perror("Failed to execute exit path");
        return 1; // Exit if exec fails
    }

    // Check if the admin is already logged in
    if (strcmp(a.loggedin, "y") == 0) {
        write(STDOUT_FILENO, "User already logged in another session\n", 39);
        unlock_file(fd);
        close(fd);
        execvp(ExitPath, NULL); // Execute exit path
        perror("Failed to execute exit path");
        return 1; // Exit if exec fails
    }

    write(STDOUT_FILENO, "Enter password: ", 16);
    read_password(password, sizeof(password)); // Call the new password read function

    // Verify the entered password against the stored hash
    if (crypto_pwhash_str_verify(a.hashed_password, password, strlen(password)) != 0) {
        write(STDOUT_FILENO, "\nInvalid password\n", 18);
        unlock_file(fd);
        close(fd);
        execvp(ExitPath, NULL); // Execute exit path
        perror("Failed to execute exit path");
        return 1; // Exit if exec fails
    } else {
        write(STDOUT_FILENO, "\nLogin successful\n", 18);
        a.loggedin[0] = 'y'; // Update logged-in status
        a.loggedin[1] = '\0';

        // Move the file pointer to the correct position to update the record
        if (lseek(fd, -sizeof(a), SEEK_CUR) == (off_t)-1) {
            perror("Failed to seek to user position");
            unlock_file(fd);
            close(fd);
            return 1; // Exit if seeking fails
        }

        // Write the updated admin login struct
        if (write(fd, &a, sizeof(a)) != sizeof(a)) {
            perror("Failed to write updated login status");
            unlock_file(fd);
            close(fd);
            return 1; // Exit if write fails
        }

        write(STDOUT_FILENO, "Updated login status for user: ", 31);
        write(STDOUT_FILENO, a.username, strlen(a.username));
        write(STDOUT_FILENO, "\n", 1);
    }

    // Unlock the whole file before executing manager actions
    unlock_file(fd);
    close(fd); // Close the file descriptor before executing manager actions

    // Execute manager actions
    char *args[] = {username, NULL};
    execvp(ManagerActionsPath, args);
    
    // If execvp fails
    perror("Failed to execute manager actions");
    return 1; // Exit if execvp fails
}
