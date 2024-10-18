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

int main() {
    // Initialize libsodium
    if (sodium_init() < 0) {
        return 1; // Panic! The library couldn't be initialized
    }

    char CustomerLoginsPath[256];
    snprintf(CustomerLoginsPath, sizeof(CustomerLoginsPath), "%s%s", basePath, "/customer/customerlogins.txt");

    char CustomerActionsPath[256];
    snprintf(CustomerActionsPath, sizeof(CustomerActionsPath), "%s%s", basePath, "/customer/customer.out");

    int fd = open(CustomerLoginsPath, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open customer logins file");
        return 1;
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
            break;
        }
        pos += sizeof(e); // Track the position of the current record
    }

    if (!found) {
        printf("User doesn't exist\n");
        close(fd);
        return 0;
    }

    if (strcmp(e.loggedin, "y") == 0) {
        printf("User already logged in one session\n");
        close(fd);
        return 0;
    }

    printf("Enter your password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Verify the password
    if (crypto_pwhash_str_verify(e.hashed_password, password, strlen(password)) != 0) {
        printf("Invalid password\n");
        close(fd);
        return 0;
    } else {
        printf("Login successful\n");
        e.loggedin[0] = 'y';
        e.loggedin[1] = '\0';

        // Open file for reading and writing
        int fd2 = open(CustomerLoginsPath, O_RDWR);
        if (fd2 < 0) {
            perror("Failed to open logins file for writing");
            close(fd);
            return 1;
        }

        // Move the file pointer to the correct position to update the record
        if (lseek(fd2, pos, SEEK_SET) == -1) {
            perror("Failed to seek to the correct position");
            close(fd2);
            close(fd);
            return 1;
        }

        // Write the updated record
        if (write(fd2, &e, sizeof(e)) != sizeof(e)) {
            perror("Failed to write updated login status");
            close(fd2);
            close(fd);
            return 1;
        }

        close(fd2);
        close(fd);

        // Execute customer actions
        char* args[] = {e.username, NULL};
        execvp(CustomerActionsPath, args);
        perror("execvp failed");
    }

    return 0;
}
