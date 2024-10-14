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

    char ManagerLoginsPath[256];
    snprintf(ManagerLoginsPath, sizeof(ManagerLoginsPath), "%s%s", basePath, "/manager/managerlogins.txt");

    int fd;
    struct AdminLogin a;
    int found = 0;
    char username[50], password[50];

    // Ask for username and password input
    printf("Welcome to the admin dashboard\n");
    printf("Please login to proceed further\n");
    printf("Enter username\n");
    fgets(username, sizeof(username), stdin);
    remove_newline(username);
    
    printf("Enter password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Open the file containing the manager logins
    fd = open(ManagerLoginsPath, O_RDONLY);
    if (fd == -1) {
        printf("Failed to open the login file\n");
        return 1;
    }

    // Search for the entered username and verify the password
    while (read(fd, &a, sizeof(a)) > 0) {
        if (strcmp(a.username, username) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("User doesn't exist\n");
        close(fd);
        return 0;
    }

    // Verify the entered password against the stored hash
    if (crypto_pwhash_str_verify(a.hashed_password, password, strlen(password)) != 0) {
        printf("Invalid password\n");
    } else {
        printf("Login successful\n");
    }

    close(fd);
    return 0;
}
