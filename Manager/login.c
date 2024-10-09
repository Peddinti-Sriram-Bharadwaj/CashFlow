#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(){
  printf("welcome to the admin dashboard\n");
  printf("Please login to proceed further\n");
  printf("Enter username\n");
  char username[50];
  scanf("%s", username);
  printf("Enter password\n");
  char password[50];
  scanf("%s", password);
  printf("logged in successfully\n");

  return 0;
}
