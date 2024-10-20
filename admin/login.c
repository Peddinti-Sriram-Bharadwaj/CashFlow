#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include <termios.h> // Include for termios functionality
#include "../global.c"

struct AdminLogin {
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
        // If the lock is unavailable, prompt the user to wait
        const char *msg = "Lock is currently unavailable. Please wait...\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        sleep(1); // Wait for a second before retrying
    }
    return 0; // Return success
}

// Function to securely read a password
void read_password(char *password, size_t size) {
    struct termios oldt, newt;
    
    // Get the current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt; // Copy current settings
    newt.c_lflag &= ~(ECHO); // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Set new settings
    
    // Read password
    read(STDIN_FILENO, password, size);
    remove_newline(password); // Remove newline character
    
    // Restore the old terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int main() {
    // Initialize libsodium
    char AdminLoginsPath[256];
    snprintf(AdminLoginsPath, sizeof(AdminLoginsPath), "%s%s", basePath, "/admin/adminlogins.txt");

    char AdminActionsPath[256];
    snprintf(AdminActionsPath, sizeof(AdminActionsPath), "%s%s", basePath, "/admin/admin.out");

    char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");
    
    if (sodium_init() < 0) {
        const char *msg = "libsodium initialization failed\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        execl(ExitPath, ExitPath, (char *)NULL); // Exit if initialization fails
    }

    int fd = open(AdminLoginsPath, O_RDWR); // Open for reading and writing
    if (fd < 0) {
        perror("Failed to open logins file");
        execl(ExitPath, ExitPath, (char *)NULL); // Exit if open fails
    }

    // Attempt to acquire a write lock on the whole file
    if (acquire_lock(fd, F_WRLCK, 0, 0) == -1) {
        close(fd);
        execl(ExitPath, ExitPath, (char *)NULL); // Exit if lock acquisition fails
    }

    struct AdminLogin a;
    int found = 0;
    char username[50], password[50];
    off_t user_position = -1; // To track the position of the found user

    const char *welcome_msg = "****************************************\n"
                              "* Welcome to the admin dashboard       *\n"
                              "****************************************\n"
                              "Please login to proceed further\n"
                              "Enter username: ";
    write(STDOUT_FILENO, welcome_msg, strlen(welcome_msg));
    read(STDIN_FILENO, username, sizeof(username));
    remove_newline(username);

    // Search for the user and record their position
    while (read(fd, &a, sizeof(a)) > 0) {
        // Compare the current username with the input
        if (strcmp(a.username, username) == 0) {
            found = 1;
            user_position = lseek(fd, -sizeof(a), SEEK_CUR); // Record position if found
            break; // Exit the loop if user is found
        }
    }

    // If user not found, unlock the whole file and exit
    if (!found) {
        const char *msg = "****************************************\n"
                          "* User doesn't exist                   *\n"
                          "****************************************\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        execl(ExitPath, ExitPath, (char *)NULL); // Exit if user not found
    }

    // Release the write lock on the whole file
    struct flock unlock_read;
    memset(&unlock_read, 0, sizeof(unlock_read));
    unlock_read.l_type = F_UNLCK; // Unlock the whole file
    fcntl(fd, F_SETLK, &unlock_read);

    // Lock the specific user record for the found user
    if (acquire_lock(fd, F_WRLCK, user_position, sizeof(a)) == -1) {
        const char *msg = "****************************************\n"
                          "* Failed to lock user record           *\n"
                          "****************************************\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        close(fd);
        execl(ExitPath, ExitPath, (char *)NULL); // Exit if lock acquisition fails
    }

    // Check if the user is already logged in
    if (strcmp(a.loggedin, "y") == 0) {
        const char *msg = "****************************************\n"
                          "* User is already logged in one session*\n"
                          "****************************************\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        // Unlock the specific record before executing the admin actions
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the specific record
        fcntl(fd, F_SETLK, &unlock);
        close(fd); // Close the file descriptor before executing admin actions

        // Execute the admin actions
        execl(ExitPath, ExitPath, (char *)NULL); // Pass control to admin actions
        perror("Failed to execute admin actions");
    }

    // Prompt for the password if the user is not logged in
    const char *password_msg = "Enter the password: ";
    write(STDOUT_FILENO, password_msg, strlen(password_msg));
    read_password(password, sizeof(password)); // Use read_password to securely read input

    // Verify the password against the stored hash
    if (crypto_pwhash_str_verify(a.hashed_password, password, strlen(password)) != 0) {
        const char *msg = "****************************************\n"
                          "* Invalid password                     *\n"
                          "****************************************\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        // Unlock the specific record before exiting
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the specific record
        fcntl(fd, F_SETLK, &unlock);
        close(fd); // Close the file descriptor
        execl(ExitPath, ExitPath, (char *)NULL); // Exit if password verification fails
    } else {
        const char *msg = "****************************************\n"
                          "* Login successful                     *\n"
                          "****************************************\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        a.loggedin[0] = 'y'; // Update logged-in status
        a.loggedin[1] = '\0';

        // Move the file pointer to the position of the user
        if (lseek(fd, user_position, SEEK_SET) == (off_t)-1) {
            perror("Failed to seek to user position");
            // Unlock the specific record before closing
            struct flock unlock;
            memset(&unlock, 0, sizeof(unlock));
            unlock.l_type = F_UNLCK; // Unlock the specific record
            fcntl(fd, F_SETLK, &unlock);
            close(fd);
            execl(ExitPath, ExitPath, (char *)NULL); // Exit if seeking fails
        }

        // Write the updated admin login struct
        ssize_t bytes_written = write(fd, &a, sizeof(a));
        if (bytes_written != sizeof(a)) {
            perror("Failed to write updated login status");
            // Unlock the specific record before closing
            struct flock unlock;
            memset(&unlock, 0, sizeof(unlock));
            unlock.l_type = F_UNLCK; // Unlock the specific record
            fcntl(fd, F_SETLK, &unlock);
            close(fd);
            execl(ExitPath, ExitPath, (char *)NULL); // Exit if write fails
        }

        const char *update_msg = "****************************************\n"
                                 "* Updated login status for user:       *\n"
                                 "****************************************\n";
        write(STDOUT_FILENO, update_msg, strlen(update_msg));
        write(STDOUT_FILENO, a.username, strlen(a.username));
        write(STDOUT_FILENO, "\n", 1);

        // Unlock the specific record before executing the admin actions
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the specific record
        fcntl(fd, F_SETLK, &unlock);
        close(fd); // Close the file descriptor before executing admin actions

        // Execute the admin actions
        char *args[] = {username, NULL};
        execvp(AdminActionsPath, args);

        // If execvp fails
        perror("Failed to execute admin actions");
        execl(ExitPath, ExitPath, (char *)NULL); // Exit if execvp fails
    }

    // This line may not be reached if execvp is successful
    struct flock unlock;
    memset(&unlock, 0, sizeof(unlock));
    unlock.l_type = F_UNLCK; // Unlock the whole file
    fcntl(fd, F_SETLK, &unlock);
    close(fd);
    return 0;
}
