#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

struct EmployeeLogin{
  char username[20];
  char password[20];
};

int main(){
    printf("welcome to Cashflow dear user\n");
    printf("please login to proceed further\n");
    printf("Enter the username\n");
    char username[50];
    scanf("%s", username);
    printf("Enter the password\n");
    char password[50];
    scanf("%s", password);
    int fd = open("./customer/customerlogins.txt", O_RDONLY, 0644);
    printf("%d\n", fd);
    if(fd == -1) printf("not opened");
    struct EmployeeLogin e;
    int found = 0;

    while(read(fd, &e, sizeof(e)) > 0){
      if(strcmp(e.username, username) != 0){
        continue;
      }
      found = 1;
      if(strcmp(e.password,password) == 0){
      printf("login successful\n");
      }
      else{
        printf("invalid password\n");
      
      }
    }

    if(!found) printf("user doesnt exist\n");
    close(fd);
    return 0;
}
