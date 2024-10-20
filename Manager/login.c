#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
        printf("Lock is currently unavailable. Please wait...\n");
        sleep(1); // Wait for a second before retrying
    }
    return 0; // Return success
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
    if (acquire_lock(fd, F_WRLCK, 0, 0) == -1) {
        close(fd);
        return 1; // Exit if lock acquisition fails
    }

    struct AdminLogin a;
    int found = 0;
    char username[50], password[50];

    // Ask for username and password input
    printf("Welcome to the admin dashboard\n");
    printf("Please login to proceed further\n");
    printf("Enter username\n");
    fgets(username, sizeof(username), stdin);
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
        printf("User doesn't exist\n");
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        
        execvp(ExitPath, NULL); // Execute exit path
        perror("Failed to execute exit path");
        return 1; // Exit if exec fails
    }

    // Check if the admin is already logged in
    if (strcmp(a.loggedin, "y") == 0) {
        printf("User already logged in another session\n");
        
        // Unlock the whole file before executing exit path
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        
        execvp(ExitPath, NULL); // Execute exit path
        perror("Failed to execute exit path");
        return 1; // Exit if exec fails
    }

    printf("Enter password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Verify the entered password against the stored hash
    if (crypto_pwhash_str_verify(a.hashed_password, password, strlen(password)) != 0) {
        printf("Invalid password\n");
        
        // Unlock the whole file before executing exit path
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        
        execvp(ExitPath, NULL); // Execute exit path
        perror("Failed to execute exit path");
        return 1; // Exit if exec fails
    } else {
        printf("Login successful\n");
        a.loggedin[0] = 'y'; // Update logged-in status
        a.loggedin[1] = '\0';

        // Move the file pointer to the correct position to update the record
        if (lseek(fd, -sizeof(a), SEEK_CUR) == (off_t)-1) {
            perror("Failed to seek to user position");
            struct flock unlock;
            memset(&unlock, 0, sizeof(unlock));
            unlock.l_type = F_UNLCK; // Unlock the whole file
            fcntl(fd, F_SETLK, &unlock);
            close(fd);
            return 1; // Exit if seeking fails
        }

        // Write the updated admin login struct
        if (write(fd, &a, sizeof(a)) != sizeof(a)) {
            perror("Failed to write updated login status");
            struct flock unlock;
            memset(&unlock, 0, sizeof(unlock));
            unlock.l_type = F_UNLCK; // Unlock the whole file
            fcntl(fd, F_SETLK, &unlock);
            close(fd);
            return 1; // Exit if write fails
        }

        printf("Updated login status for user: %s\n", a.username);

        // Unlock the whole file before executing manager actions
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd); // Close the file descriptor before executing manager actions

        // Execute manager actions
        char *args[] = {username, NULL};
        execvp(ManagerActionsPath, args);
        
        // If execvp fails
        perror("Failed to execute manager actions");
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
