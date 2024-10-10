#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
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
    char EmployeeLoginsPath[256];
    snprintf(EmployeeLoginsPath, sizeof(EmployeeLoginsPath), "%s%s", basePath, "/customer/customerlogins.txt"); 

    int fd = open(EmployeeLoginsPath, O_RDONLY, 0644);
    struct EmployeeLogin e;
    int found = 0;
    char username[50];
    char password[50];

    printf("welcome to cashflow dear customer\n");
    printf("Please login below to proceed further\n");
    printf("Enter your username\n");
    fgets(username,20,stdin);
    remove_newline(username);
     

    while(read(fd, &e, sizeof(e)) > 0){
      if(strcmp(e.username, username) != 0){
        continue;
      }
      found = 1;
      break;
    }

    if(!found){
      printf("user doesnt exist\n");
      return 0;
    }

    printf("Enter the password\n");
    fgets(password,20,stdin);
    remove_newline(password);

    if(strcmp(e.password,password) == 0){
      printf("login successful\n");
      }
    else{
      printf("invalid password\n");     
      }

    close(fd);
    return 0;
}
