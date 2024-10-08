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
    EmployeeCredentials emp;
    int fd;
    ssize_t bytes_written;

    write(STDOUT_FILENO, "Enter username: ", 16);
    read(STDIN_FILENO, emp.username, sizeof(emp.username) - 1);
    emp.username[sizeof(emp.username) - 1] = '\0';
    write(STDOUT_FILENO, "Enter password: ", 16);
    read(STDIN_FILENO, emp.password, sizeof(emp.password) - 1);
    emp.password[sizeof(emp.password) - 1] = '\0';

    fd = open("/Users/srirambharadwaj/Documents/iiitb/sem1/ss/miniproject/employee/employee_records.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    bytes_written = write(fd, &emp, sizeof(emp));
    if (bytes_written == -1) {
        perror("Error writing to file");
        close(fd);
        return 1;
    }

    close(fd);

    write(STDOUT_FILENO, "Employee account created successfully.\n", 39);
    //execute the login page of the created account now. 
    
    return 0;
}