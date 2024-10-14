#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"

int main(){
  char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");

  printf("Welcome to the admin dashboard\n");
  printf("Please choose one of the options to proceed further\n");
  printf("add new bank employee -1\n");
  printf("modify customer details -2\n");
  printf("manage user roles -3\n");
  printf("change password -4\n");
  printf("Logout -5\n");
  printf("Exit -6\n");

  int option;
  scanf("%d", &option);

  switch(option){
    case 1:
      printf("add new bank employee\n");
      break;
    case 2:
      printf("modify customer details\n");
      break;
    case 3:
      printf("manage user roles\n");
      break;
    case 4:
      printf("change password\n");
      break;
    case 5:
      printf("logout\n");
      break;
    case 6:
      printf("Exit\n");
      execvp(ExitPath, NULL); 
      break;
    }

  return 0;
}
