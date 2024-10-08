#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
  printf("welcome to CashFlow dear customer\n");
  printf("Plese choose one of the below options to proceed further\n");
  printf("Sign up\n");
  printf("Login\n");
  int option;
  scanf("%d", &option);
  switch(option){
    case 1:
      printf("signup\n");
      break;
    case 2:
      printf("login\n");
      break;
    default:
      printf("invalid option\n");
  }
  return 0;
}
