#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"

int main(int argc, char *argv[]) {

    char *username = argv[0]; // Correctly retrieve the username
    char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");
    
    char LogOutPath[256];
    snprintf(LogOutPath, sizeof(LogOutPath), "%s%s", basePath, "/admin/logout.out");

    printf("Welcome to the admin dashboard\n");
    printf("Hello %s\n", username);
    printf("Please choose one of the options to proceed further\n");
    printf("Add new bank employee - 1\n");
    printf("Modify customer details - 2\n");
    printf("Manage user roles - 3\n");
    printf("Change password - 4\n");
    printf("Logout - 5\n");
    printf("Exit - 6\n");

    int option;
    scanf("%d", &option);

    switch(option) {
        case 1:
            printf("Add new bank employee\n");
            break;
        case 2:
            printf("Modify customer details\n");
            break;
        case 3:
            printf("Manage user roles\n");
            break;
        case 4:
            printf("Change password\n");
            break;
        case 5:
            printf("Logout\n");
            {
                // Prepare arguments for execvp
                char *args[] = {username, NULL}; // First element is path, second is username
                printf("Calling logout...\n");
                execvp(LogOutPath, args); 
                perror("execvp failed"); // Handle execvp failure
                return 1; // Exit if execvp fails
            }
            break;
        case 6:
            printf("Exit\n");
            execvp(ExitPath, NULL);
            perror("execvp failed"); // Handle execvp failure
            return 1; // Exit if execvp fails
            break;
        default:
            printf("Invalid option. Please try again.\n");
            break;
    }

    return 0;
}