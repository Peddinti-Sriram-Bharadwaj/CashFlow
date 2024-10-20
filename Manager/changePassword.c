#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"

struct ManagerLogin {
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

int main(int argc, char *argv[]) {
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

    struct ManagerLogin a;
    int found = 0;
    char old_password[20], new_password[20], confirm_password[20];

    printf("Welcome to the manager dashboard\n");
    printf("Please login below to proceed further\n");
    char *username = argv[0];
    printf("Hello %s\n", username);

    // Search for the user in the file
    off_t pos = 0;
    while (read(fd, &a, sizeof(a)) > 0) {
        if (strcmp(a.username, username) == 0) {
            found = 1;
            break; // Exit the loop if user is found
        }
        pos += sizeof(a); // Track the position of the current record
    }

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

    printf("Enter your old password\n");
    fgets(old_password, sizeof(old_password), stdin);
    remove_newline(old_password);

    // Verify the old password
    if (crypto_pwhash_str_verify(a.hashed_password, old_password, strlen(old_password)) != 0) {
        printf("Invalid old password\n");
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        execvp(ExitPath, NULL); // Execute exit path
        perror("Failed to execute exit path");
        return 1; // Exit if exec fails
    }

    printf("Enter your new password\n");
    fgets(new_password, sizeof(new_password), stdin);
    remove_newline(new_password);

    printf("Re-enter your new password for confirmation\n");
    fgets(confirm_password, sizeof(confirm_password), stdin);
    remove_newline(confirm_password);

    if (strcmp(new_password, confirm_password) != 0) {
        printf("Passwords do not match. Try again.\n");
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        execvp(ExitPath, NULL); // Execute exit path
        perror("Failed to execute exit path");
        return 1; // Exit if exec fails
    }

    // Hash the new password
    char hashed_new_password[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(hashed_new_password, new_password, strlen(new_password), 
                           crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Password hashing failed\n");
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        return 1;
    }

    // Update the password in the struct
    memcpy(a.hashed_password, hashed_new_password, crypto_pwhash_STRBYTES);

    // Move the file pointer to the correct position to update the record
    if (lseek(fd, pos, SEEK_SET) == -1) {
        perror("Failed to seek to the correct position");
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        return 1; // Exit if seeking fails
    }

    // Write the updated record
    if (write(fd, &a, sizeof(a)) != sizeof(a)) {
        perror("Failed to write updated password");
        struct flock unlock;
        memset(&unlock, 0, sizeof(unlock));
        unlock.l_type = F_UNLCK; // Unlock the whole file
        fcntl(fd, F_SETLK, &unlock);
        close(fd);
        return 1; // Exit if write fails
    }

    printf("Password updated successfully\n");
    
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
