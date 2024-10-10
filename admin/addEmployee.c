#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "../global.c"

struct EmployeeLogin{
  char username[20];
  char password[20];
};

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

int main(){
  printf("please enter the name of the Employee to be added\n");
  printf("Enter the username\n");
  char username[20], password[20];
  fgets(username,20,stdin);
  remove_newline(username);
  
  printf("Ask the employee to create password\n");
  fgets(password,20,stdin); 
  remove_newline(password);
  struct EmployeeLogin e1;
  strcpy(e1.username, username);
  strcpy(e1.password, password);
  char EmployeeLoginsPath[256];
  snprintf(EmployeeLoginsPath,sizeof(EmployeeLoginsPath),"%s%s", basePath, "/employee/employeelogins.txt"); 
  int fd = open(EmployeeLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
  if(fd == -1) printf("not opened");
  write(fd, &e1, sizeof(e1));

  close(fd);

  
  return 0;
}

