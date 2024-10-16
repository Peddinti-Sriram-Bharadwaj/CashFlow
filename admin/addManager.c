#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"

struct ManagerLogin{
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

int main(){
    // Initialize libsodium
    if (sodium_init() < 0) {
        printf("libsodium initialization failed\n");
        return 1;
    }

    printf("Please enter the name of the Manager to be added\n");
    printf("Enter the username\n");
    char username[20], password[20];
    fgets(username, 20, stdin);
    remove_newline(username);
  
    printf("Ask the Manager to create password\n");
    fgets(password, 20, stdin); 
    remove_newline(password);

    // Hash the password
    struct ManagerLogin e;
    if (crypto_pwhash_str(e.hashed_password, password, strlen(password), crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        printf("Password hashing failed\n");
        return 1;
    }

    // Store the username and hashed password
    strncpy(e.username, username, sizeof(e.username) - 1); // Ensure null-termination
    strncpy(e.loggedin, "n", sizeof(e.loggedin));

    // Write to the manager login file
    char ManagerLoginsPath[256];
    snprintf(ManagerLoginsPath, sizeof(ManagerLoginsPath), "%s%s", basePath, "/Manager/managerlogins.txt"); 
    int fd = open(ManagerLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        printf("Failed to open the file\n");
        return 1;
    }

    // Write the struct with username and hashed password to the file
    write(fd, &e, sizeof(e));
    close(fd);

    printf("Manager account created successfully\n");

    return 0;
}
