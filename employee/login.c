#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include <termios.h> // Include termios for password input
#include "../global.c"

struct EmployeeLogin {
    char username[20];
    char loggedin[2];
    char hashed_password[crypto_pwhash_STRBYTES]; // Store the hashed password
};

// Function to remove newline character from input
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

// Function to unlock the file and close the descriptor
void unlock_and_close(int fd) {
    struct flock unlock;
    memset(&unlock, 0, sizeof(unlock));
    unlock.l_type = F_UNLCK; // Unlock the whole file
    fcntl(fd, F_SETLK, &unlock);
    close(fd);
}

// Function to read password securely and echo '*' for each character
void read_password(char *password, size_t size) {
    struct termios oldt, newt;
    char ch;
    size_t i = 0;

    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO); // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Set new settings

    // Read password and echo '*' for each character
    while (i < size - 1 && read(STDIN_FILENO, &ch, 1) > 0) {
        if (ch == '\n' || ch == '\r') {
            break; // Exit on newline
        }
        password[i++] = ch; // Store character in password array
        write(STDOUT_FILENO, "*", 1); // Echo '*' for the character
    }
    password[i] = '\0'; // Null-terminate the password

    // Restore old terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    write(STDOUT_FILENO, "\n", 1); // New line after password input
}

int main() {
    // Initialize libsodium
    if (sodium_init() < 0) {
        write(STDOUT_FILENO, "Failed to initialize libsodium\n", 32);
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
        unlock_and_close(fd);
        return 1; // Exit if lock acquisition fails
    }

    struct EmployeeLogin e;
    int found = 0;
    char username[50], password[50];

    write(STDOUT_FILENO, "========================================\n", 41);
    write(STDOUT_FILENO, "Welcome to the Employee dashboard\n", 34);
    write(STDOUT_FILENO, "Please login to proceed further\n", 32);
    write(STDOUT_FILENO, "========================================\n", 41);
    write(STDOUT_FILENO, "Enter your username: ", 21);
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
        write(STDOUT_FILENO, "========================================\n", 41);
        write(STDOUT_FILENO, "User doesn't exist\n", 19);
        write(STDOUT_FILENO, "========================================\n", 41);
        unlock_and_close(fd);
        return 0; // Exit if user not found
    }

    // Check if the employee is already logged in
    if (strcmp(e.loggedin, "y") == 0) {
        write(STDOUT_FILENO, "========================================\n", 41);
        write(STDOUT_FILENO, "User already logged in another session\n", 40);
        write(STDOUT_FILENO, "========================================\n", 41);
        unlock_and_close(fd);
        execvp(ExitPath, NULL); // Execute exit path
        perror("Failed to execute exit path");
        return 1; // Exit if exec fails
    }

    // Prompt for the password
    write(STDOUT_FILENO, "Enter your password: ", 20);
    read_password(password, sizeof(password)); // Securely read password

    // Verify the password
    if (crypto_pwhash_str_verify(e.hashed_password, password, strlen(password)) != 0) {
        write(STDOUT_FILENO, "========================================\n", 41);
        write(STDOUT_FILENO, "Invalid password\n", 17);
        write(STDOUT_FILENO, "========================================\n", 41);
        unlock_and_close(fd); // Unlock and close if password verification fails
        return 0; // Exit if password verification fails
    } else {
        write(STDOUT_FILENO, "========================================\n", 41);
        write(STDOUT_FILENO, "Login successful\n", 17);
        write(STDOUT_FILENO, "========================================\n", 41);
        e.loggedin[0] = 'y'; // Update logged-in status
        e.loggedin[1] = '\0';

        // Move the file pointer to the position of the user
        if (lseek(fd, user_position, SEEK_SET) == (off_t)-1) {
            perror("Failed to seek to user position");
            unlock_and_close(fd); // Unlock and close if seeking fails
            return 1; // Exit if seeking fails
        }

        // Write the updated employee login struct
        if (write(fd, &e, sizeof(e)) != sizeof(e)) {
            perror("Failed to write updated login status");
            unlock_and_close(fd); // Unlock and close if write fails
            return 1; // Exit if write fails
        }

        write(STDOUT_FILENO, "Updated login status for user: ", 31);
        write(STDOUT_FILENO, e.username, strlen(e.username));
        write(STDOUT_FILENO, "\n", 1);

        // Unlock the whole file before executing employee actions
        unlock_and_close(fd);

        // Execute the employee actions
        char *args[] = {username, NULL};
        execvp(EmployeeActionsPath, args);
        
        // If execvp fails
        perror("Failed to execute employee actions");
        return 1; // Exit if execvp fails
    }

    // This line may not be reached if execvp is successful
    unlock_and_close(fd); // Ensure the file is unlocked and closed
    return 0;
}
