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
    char AdminActionsPath[256];
    snprintf(AdminActionsPath, sizeof(AdminActionsPath), "%s%s", basePath, "/admin/admin.out");
    char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");
    
    char LogOutPath[256];
    snprintf(LogOutPath, sizeof(LogOutPath), "%s%s", basePath, "/admin/logout.out");

    char AddEmployeePath[256];
    snprintf(AddEmployeePath, sizeof(AddEmployeePath), "%s%s", basePath, "/admin/addEmployee.out");

    char AddManagerPath[256];
    snprintf(AddManagerPath, sizeof(AddManagerPath), "%s%s", basePath, "/admin/addManager.out");

    char changePasswordPath[256];
    snprintf(changePasswordPath, sizeof(changePasswordPath), "%s%s", basePath, "/admin/changePassword.out");

    // Using write system call instead of printf
    write(STDOUT_FILENO, "========================================\n", 41);
    write(STDOUT_FILENO, "Welcome to the admin dashboard\n", 31);
    write(STDOUT_FILENO, "========================================\n", 41);
    write(STDOUT_FILENO, "Hello ", 6);
    write(STDOUT_FILENO, username, strlen(username));
    write(STDOUT_FILENO, "\n========================================\n", 42);
    write(STDOUT_FILENO, "Please choose one of the options to proceed further\n", 54);
    write(STDOUT_FILENO, "========================================\n", 41);
    write(STDOUT_FILENO, "1. Add new bank employee\n", 25);
    write(STDOUT_FILENO, "2. Add a new Manager\n", 21);
    write(STDOUT_FILENO, "3. Modify customer details\n", 27);
    write(STDOUT_FILENO, "4. Manage user roles\n", 21);
    write(STDOUT_FILENO, "5. Change password\n", 19);
    write(STDOUT_FILENO, "6. Logout\n", 10);
    write(STDOUT_FILENO, "========================================\n", 41);

    int option;
    char buffer[4];
    read(STDIN_FILENO, buffer, sizeof(buffer));
    option = atoi(buffer);

    switch(option) {
        case 1:
            write(STDOUT_FILENO, "Add new bank employee\n", 22);
            execvp(AddEmployeePath, argv);
            break;
        case 2:
            write(STDOUT_FILENO, "Add a new Manager\n", 18);
            execvp(AddManagerPath, argv);
            break;
        case 3:
            write(STDOUT_FILENO, "Modify customer details\n", 24);
            printf("This feature is not yet implemented\n");
            execvp(AdminActionsPath, argv);
            break;
        case 4:
            write(STDOUT_FILENO, "Manage user roles\n", 18);
            printf("This feature is not yet implemented\n");
            execvp(AdminActionsPath, argv);
            break;
        case 5:
            write(STDOUT_FILENO, "Change password\n", 16);
            execvp(changePasswordPath, argv);
            break;
        case 6:
            write(STDOUT_FILENO, "Logout\n", 7);
            {
                // Prepare arguments for execvp
                char *args[] = {username, NULL}; // First element is path, second is username
                write(STDOUT_FILENO, "Calling logout...\n", 18);
                execvp(LogOutPath, args); 
                perror("execvp failed"); // Handle execvp failure
                return 1; // Exit if execvp fails
            }
            break;
        default:
            write(STDOUT_FILENO, "Invalid option. Please try again.\n", 34);
            break;
    }

    return 0;
}