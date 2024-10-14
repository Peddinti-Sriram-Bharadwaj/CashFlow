#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"

struct AdminLogin{
  char username[20];
  char hashed_password[crypto_pwhash_STRBYTES]; // Store the hashed password
};

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

int main(){
    // Initialize libsodium
    if (sodium_init() < 0) {
        printf("libsodium initialization failed\n");
        return 1;
    }

    printf("Please enter the name of the customer to be added\n");
    printf("Enter the username\n");
    char username[20], password[20];
    fgets(username, 20, stdin);
    remove_newline(username);

    printf("Ask the customer to create password\n");
    fgets(password, 20, stdin);
    remove_newline(password);

    // Hash the password
    struct AdminLogin a1;
    if (crypto_pwhash_str(a1.hashed_password, password, strlen(password), crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Password hashing failed\n");
        return 1;
    }

    // Store the username and hashed password in the struct
    strncpy(a1.username, username, sizeof(a1.username) - 1); // Ensure null-termination

    // Write to the admin login file
    char AdminLoginsPath[256];
    snprintf(AdminLoginsPath, sizeof(AdminLoginsPath), "%s%s", basePath, "/admin/adminlogins.txt");

    int fd = open(AdminLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        printf("Failed to open the file\n");
        return 1;
    }

    // Write the struct with username and hashed password to the file
    write(fd, &a1, sizeof(a1));
    close(fd);

    printf("Admin account created successfully\n");

    return 0;
}
