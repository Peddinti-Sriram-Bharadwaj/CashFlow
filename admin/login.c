#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int main(){
  printf("Welcome to the admin dashboard\n");
  printf("Please login to proceed further\n");
  printf("Enter username\n");
  char username[50];
  scanf("%s", username);
  printf("Enter password\n");
  char password[50];
  scanf("%s", password);
  printf("loggedin successfully");

  return 0;

}
