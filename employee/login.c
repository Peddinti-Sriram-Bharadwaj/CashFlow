#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char username[50];
    char password[50];
} EmployeeCredentials;

int main(){
    //ask user to enter username and password
    char username[50];
    char password[50];
    int fd;
    ssize_t bytes_written;
    char buffer[150];

    write(STDOUT_FILENO, "Enter username: ", 16);
    read(STDIN_FILENO, username, sizeof(username));
    write(STDOUT_FILENO, "Enter password: ", 16);
    read(STDIN_FILENO, password, sizeof(password));

    // Open the file containing usernames and passwords
    int login_fd = open("/Users/srirambharadwaj/Documents/iiitb/sem1/ss/miniproject/employee/employee_records.txt", O_RDONLY);
    if (login_fd == -1) {
        write(STDERR_FILENO, "Error opening login file\n", 25);
        return 1;
    }

    // Read the file to check if the username and password match
    EmployeeCredentials credentials;
    int attempts = 3;
    int match_found = 0;
    ssize_t bytes_read;

    while (attempts > 0) {
        lseek(login_fd, 0, SEEK_SET); // Reset file pointer to the beginning
        while ((bytes_read = read(login_fd, &credentials, sizeof(EmployeeCredentials))) > 0) {
            if (strcmp(username, credentials.username) == 0 && strcmp(password, credentials.password) == 0) {
                match_found = 1;
                break;
            }
        }

        if (match_found) {
            write(STDOUT_FILENO, "Login successful. Executing employee program...\n", 49);
            // Execute the employee program here
            // For example: system("./employee_program");
            //call the employee.out prgram using exec family of calls. pass the username as argument
            execlp("iterm", "iterm", "-e", "./employee.out", username, (char *)NULL);
            perror("execlp failed");



            break;
        } else {
            attempts--;
            if (attempts > 0) {
                write(STDOUT_FILENO, "Invalid credentials. Please try again.\n", 39);
                write(STDOUT_FILENO, "Enter username: ", 16);
                read(STDIN_FILENO, username, sizeof(username));
                write(STDOUT_FILENO, "Enter password: ", 16);
                read(STDIN_FILENO, password, sizeof(password));
            } else {
                write(STDOUT_FILENO, "Too many failed attempts. Aborting.\n", 36);
            }
        }
    }

    close(login_fd);

    return 0;
}
