#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
  printf("Welcome back to Cashflow\n");
  printf("Sign in to proceed further\n");
  printf("Enter username\n");
  char username[50];
  printf("Enter password\n");
  char password[50];
  printf("logged in successfully\n");
  

  return 0;
}
