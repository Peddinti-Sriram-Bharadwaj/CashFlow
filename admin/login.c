#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
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

int main() {
    // Initialize libsodium
    if (sodium_init() < 0) {
        printf("libsodium initialization failed\n");
        return 1;
    }

    char AdminLoginsPath[256];
    snprintf(AdminLoginsPath, sizeof(AdminLoginsPath), "%s%s", basePath, "/admin/adminlogins.txt");

    char AdminActionsPath[256];
    snprintf(AdminActionsPath, sizeof(AdminActionsPath), "%s%s", basePath, "/admin/admin.out");

    int fd = open(AdminLoginsPath, O_RDONLY);
    struct AdminLogin a;
    int found = 0;
    char username[50], password[50];

    printf("Welcome to the admin dashboard\n");
    printf("Please login to proceed further\n");
    printf("Enter username\n");
    fgets(username, sizeof(username), stdin);
    remove_newline(username);

    while (read(fd, &a, sizeof(a)) > 0) {
        if (strcmp(a.username, username) != 0) {
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

    if(strcmp(a.loggedin, "y")== 0){
        printf("User already logged in one session");
        close(fd);
        return 0;
    }

    printf("Enter the password\n");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    // Verify the password against the stored hash
    if (crypto_pwhash_str_verify(a.hashed_password, password, strlen(password)) != 0) {
        printf("Invalid password\n");
    } else {
        printf("Login successful\n");
        close(fd);
        a.loggedin[0] = 'y';
        a.loggedin[1] = '\0';

        int fd2 = open(AdminLoginsPath, O_RDWR);
        if(fd2<0){
            perror("Failed to open logins file for writing");
            return 1;
        }
        lseek(fd2, -sizeof(a), SEEK_CUR);
        
        if(write(fd2, &a, sizeof(a)) != sizeof(a)){
            perror("Failed to write updated long status");
            close(fd2);
            return 1;
        }
        execvp(AdminActionsPath, NULL);
    }

    close(fd);
    return 0;
}
