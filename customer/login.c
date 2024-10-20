#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"

struct CustomerLogin {
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

    char CustomerLoginsPath[256];
    snprintf(CustomerLoginsPath, sizeof(CustomerLoginsPath), "%s%s", basePath, "/customer/customerlogins.txt");

    char CustomerActionsPath[256];
    snprintf(CustomerActionsPath, sizeof(CustomerActionsPath), "%s%s", basePath, "/customer/customer.out");

    char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");

    int fd = open(CustomerLoginsPath, O_RDWR); // Open for reading and writing
    if (fd < 0) {
        perror("Failed to open customer logins file");
        return 1; // Exit if open fails
    }

    // Attempt to acquire a write lock on the whole file
    if (acquire_lock(fd, F_WRLCK, 0, 0) == -1) {
        close(fd);
        return 1; // Exit if lock acquisition fails
    }

    struct CustomerLogin e;
    int found = 0;
    char username[20], password[20];

    printf("Welcome to cashflow, dear customer\n");
    printf("Please login below to proceed further\n");
    printf("Enter your username\n");
    fgets(username, sizeof(username), stdin);
    remove_newline(username);

    // Search for the user in the file
    off_t pos = 0;
    while (read(fd, &e, sizeof(e)) > 0) {
        if (strcmp(e.username, username) == 0) {
            found = 1;
            break; // Exit the loop if user is found
        }
        pos += sizeof(e); // Track the position of the current record
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

    // Check if the customer is already logged in
    if (strcmp(e.loggedin, "y") == 0) {
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

    printf("Enter your password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Verify the password
    if (crypto_pwhash_str_verify(e.hashed_password, password, strlen(password)) != 0) {
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
        e.loggedin[0] = 'y'; // Update logged-in status
        e.loggedin[1] = '\0';

        // Move the file pointer to the correct position to update the record
        if (lseek(fd, pos, SEEK_SET) == (off_t)-1) {
            perror("Failed to seek to user position");
            struct flock unlock;
            memset(&unlock, 0, sizeof(unlock));
            unlock.l_type = F_UNLCK; // Unlock the whole file
            fcntl(fd, F_SETLK, &unlock);
            close(fd);
            return 1; // Exit if seeking fails
        }

        // Write the updated customer login struct
        if (write(fd, &e, sizeof(e)) != sizeof(e)) {
            perror("Failed to write updated login status");
            struct flock unlock;
            memset(&unlock, 0, sizeof(unlock));
            unlock.l_type = F_UNLCK; // Unlock the whole file
            fcntl(fd, F_SETLK, &unlock);
            close(fd);
            return 1; // Exit if write fails
        }

        printf("Updated login status for user: %s\n", e.username);

        // Unlock the whole file before executing customer actions
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd); // Close the file descriptor before executing customer actions

        // Execute customer actions
        char *args[] = {e.username, NULL};
        execvp(CustomerActionsPath, args);
        
        // If execvp fails
        perror("Failed to execute customer actions");
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
