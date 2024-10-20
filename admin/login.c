#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
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
        printf("Lock is currently unavailable. Please wait...\n");
        sleep(1); // Wait for a second before retrying
    }
    return 0; // Return success
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
        printf("libsodium initialization failed\n");
        execvp(ExitPath, NULL); // Exit if initialization fails
    }

    int fd = open(AdminLoginsPath, O_RDWR); // Open for reading and writing
    if (fd < 0) {
        perror("Failed to open logins file");
        execvp(ExitPath, NULL); // Exit if open fails
    }

    // Attempt to acquire a write lock on the whole file
    if (acquire_lock(fd, F_WRLCK, 0, 0) == -1) {
        close(fd);
        execvp(ExitPath, NULL); // Exit if lock acquisition fails
    }

    struct AdminLogin a;
    int found = 0;
    char username[50], password[50];
    off_t user_position = -1; // To track the position of the found user

    printf("Welcome to the admin dashboard\n");
    printf("Please login to proceed further\n");
    printf("Enter username\n");
    fgets(username, sizeof(username), stdin);
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
        printf("User doesn't exist\n");
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        execvp(ExitPath, NULL); // Exit if user not found
    }

    // Release the write lock on the whole file
    struct flock unlock_read;
    memset(&unlock_read, 0, sizeof(unlock_read));
    unlock_read.l_type = F_UNLCK; // Unlock the whole file
    fcntl(fd, F_SETLK, &unlock_read);

    // Lock the specific user record for the found user
    if (acquire_lock(fd, F_WRLCK, user_position, sizeof(a)) == -1) {
        printf("Failed to lock user record\n");
        close(fd);
        execvp(ExitPath, NULL); // Exit if lock acquisition fails
    }

    // Check if the user is already logged in
    if (strcmp(a.loggedin, "y") == 0) {
        printf("User is already logged in one session\n");
        // Unlock the specific record before executing the admin actions
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the specific record
        fcntl(fd, F_SETLK, &unlock);
        close(fd); // Close the file descriptor before executing admin actions

        // Execute the admin actions
        execvp(ExitPath, NULL); // Pass control to admin actions
        perror("Failed to execute admin actions");
    }

    // Prompt for the password if the user is not logged in
    printf("Enter the password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Verify the password against the stored hash
    if (crypto_pwhash_str_verify(a.hashed_password, password, strlen(password)) != 0) {
        printf("Invalid password\n");
        // Unlock the specific record before exiting
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the specific record
        fcntl(fd, F_SETLK, &unlock);
        close(fd); // Close the file descriptor
        execvp(ExitPath, NULL); // Exit if password verification fails
    } else {
        printf("Login successful\n");
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
            execvp(ExitPath, NULL); // Exit if seeking fails
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
            execvp(ExitPath, NULL); // Exit if write fails
        }

        printf("Updated login status for user: %s\n", a.username);

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
        execvp(ExitPath, NULL); // Exit if execvp fails
    }

    // This line may not be reached if execvp is successful
    struct flock unlock;
    memset(&unlock, 0, sizeof(unlock));
    unlock.l_type = F_UNLCK; // Unlock the whole file
    fcntl(fd, F_SETLK, &unlock);
    close(fd);
    return 0;
}
