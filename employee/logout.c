#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"


struct EmployeeLogin {
    char username[20];
    char loggedin[2]; // Should hold 'y' or 'n' and null terminator
    char hashed_password[crypto_pwhash_STRBYTES]; // Store the hashed password
};

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

int main(int argc, char* argv[]) {
     char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");
    // Initialize libsodium (if needed)
    if (sodium_init() < 0) {
        printf("libsodium initialization failed\n");
        return 1;
    }
    char* username = argv[0];
    printf("inside the logout\n");

    remove_newline(username);

    // Open admin logins file for reading and writing
    char EmployeeLoginsPath[256];
    snprintf(EmployeeLoginsPath, sizeof(EmployeeLoginsPath), "%s%s", basePath, "/employee/employeelogins.txt");

    int fd = open(EmployeeLoginsPath, O_RDWR);
    if (fd == -1) {
        printf("Failed to open the admin logins file\n");
        return 1;
    }

    struct EmployeeLogin a;
    int found = 0;

    // Read through the file to find the user
    while (read(fd, &a, sizeof(a)) > 0) {
        if (strcmp(a.username, username) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("User doesn't exist or is not logged in\n");
        close(fd);
        return 0;
    }

    // Update loggedin status to 'n'
    strncpy(a.loggedin, "n", sizeof(a.loggedin) - 1);
    a.loggedin[sizeof(a.loggedin) - 1] = '\0'; // Null-terminate

    // Seek back to where this user's data is stored.
    lseek(fd, -sizeof(a), SEEK_CUR); // Move back by one record size

    // Write updated struct back to file
    if (write(fd, &a, sizeof(a)) != sizeof(a)) {
        printf("Failed to write updated logout status\n");
        close(fd);
        return 1; // Exit with error
    }

    close(fd); // Close write descriptor

    printf("User logged out successfully.\n");
    execvp(ExitPath, NULL);

    return 0; // Exit successfully
}