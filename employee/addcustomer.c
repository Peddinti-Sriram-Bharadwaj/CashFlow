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

    char EmployeeLoginsPath[256];
    snprintf(EmployeeLoginsPath, sizeof(EmployeeLoginsPath), "%s%s", basePath, "/customer/customerlogins.txt");

    int fd = open(EmployeeLoginsPath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    struct CustomerLogin e;
    char username[20], password[20];

    printf("Welcome to cashflow, dear customer\n");
    printf("Please create a new account\n");
    printf("Enter your username\n");
    fgets(username, sizeof(username), stdin);
    remove_newline(username);

    printf("Enter your password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Hash the password
    if (crypto_pwhash_str(e.hashed_password, password, strlen(password), crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Password hashing failed\n");
        close(fd);
        return 1;
    }

    // Store the username and hashed password in the file
    strncpy(e.username, username, sizeof(e.username) - 1); // Ensure null-termination
    strncpy(e.loggedin, "n", sizeof(e.loggedin) - 1);
    write(fd, &e, sizeof(e));

    printf("Account created successfully\n");
    close(fd);
    return 0;
}
