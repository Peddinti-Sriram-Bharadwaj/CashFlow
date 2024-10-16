#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"

int main(int argc, char* argv[]) {
  char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");
  char LogOutPath[256];
  snprintf(LogOutPath, sizeof(LogOutPath), "%s%s", basePath, "/Manager/logout.out");

  char* username = argv[0];
  
  printf("Welcome to the admin dashboard\n");
  printf("Hello %s\n", username);
  printf("Please choose one of the below to proceed further\n");
  printf("Activate/deactivate customer accounts -1\n");
  printf("Assign loan application processes to employees -2\n");
  printf("review customer feedback -3\n");
  printf("Change password -4\n");
  printf("Logout -5\n");
  printf("Exit -6\n");

  int option;
  scanf("%d", &option);
  
  switch(option){
    case 1: 
      printf("activate/deactivate customer accounts\n");
      break;
    case 2:
      printf("assign loan application\n");
      break;
    case 3:
      printf("review customer feedback\n");
      break;
    case 4:
      printf("change password\n");
      break;
    case 5:
      printf("Logout\n");
      char* args[] = {username, NULL};
      execvp(LogOutPath, args);
      break;
    case 6:
      printf("Exit\n");
      execvp(ExitPath, NULL);
      break;
    default:
      printf("invalid option\n");
      break;
    }
  return 0;



  return 0;
}
