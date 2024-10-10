#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

struct EmployeeLogin{
  char username[20];
  char password[20];
};

int main(){
  printf("please enter the name of the customer to be added\n");
  printf("Enter the username\n");
  char username[20], password[20];
  scanf("%s", username);
  printf("Ask the customer to create password\n");
  scanf("%s", password); 
  struct EmployeeLogin e1;
  strcpy(e1.username, username);
  strcpy(e1.password, password);
  int fd = open("../customer/customerlogins.txt", O_RDWR | O_APPEND | O_CREAT, 0644);
  if(fd == -1) printf("not opened");

  int nw = write(fd, &e1, sizeof(e1));
  printf("%d\n",nw);
  struct EmployeeLogin e2;
  lseek(fd,0,SEEK_SET);
  int nr = read(fd,&e2, sizeof(e2));
  printf("%d\n",nr);
  printf("%s\n", e2.username);
  printf("%s\n", e2.password);
  close(fd);

  
  return 0;
}
