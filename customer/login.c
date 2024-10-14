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

    char EmployeeLoginsPath[256];
    snprintf(EmployeeLoginsPath, sizeof(EmployeeLoginsPath), "%s%s", basePath, "/customer/customerlogins.txt");

    int fd = open(EmployeeLoginsPath, O_RDONLY);
    struct CustomerLogin e;
    int found = 0;
    char username[20], password[20];

    printf("Welcome to cashflow, dear customer\n");
    printf("Please login below to proceed further\n");
    printf("Enter your username\n");
    fgets(username, sizeof(username), stdin);
    remove_newline(username);

    while (read(fd, &e, sizeof(e)) > 0) {
        if (strcmp(e.username, username) != 0) {
            continue;
        }
        found = 1;
        break;
    }

    if (!found) {
        printf("User doesn't exist\n");
        close(fd);
        return 0;
    }

    printf("Enter your password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Verify the password
    if (crypto_pwhash_str_verify(e.hashed_password, password, strlen(password)) != 0) {
        printf("Invalid password\n");
    } else {
        printf("Login successful\n");
    }

    close(fd);
    return 0;
}
