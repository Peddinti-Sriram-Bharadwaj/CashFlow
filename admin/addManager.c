#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "../global.c"

struct ManagerLogin{
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
  printf("please enter the name of the Manager to be added\n");
  printf("Enter the username\n");
  char username[20], password[20];
  fgets(username,20,stdin);
  remove_newline(username);
  
  printf("Ask the Manager to create password\n");
  fgets(password,20,stdin); 
  remove_newline(password);
  struct ManagerLogin e1;
  strcpy(e1.username, username);
  strcpy(e1.password, password);
  char ManagerLoginsPath[256];
  snprintf(ManagerLoginsPath,sizeof(ManagerLoginsPath),"%s%s", basePath, "/customer/customerlogins.txt"); 
  int fd = open(ManagerLoginsPath, O_RDWR | O_APPEND | O_CREAT, 0644);
  if(fd == -1) printf("not opened");
  write(fd, &e1, sizeof(e1));

  close(fd);

  
  return 0;
}

