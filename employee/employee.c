#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(){
  printf("Welcome to the workspace dear employee\n");
  printf("Select one of the following options to proceed further\n");
  printf("Add new customer -1\n");
  printf("Modify customer details -2\n");
  printf("Process loan applications -3\n");
  printf("Approve/reject loan applications -4\n");
  printf("View customer transactions -5\n");
  printf("Change password -6\n");
  printf("Logout -7\n");
  printf("Exit -8\n");

  int option;
  scanf("%d", &option);

  switch(option){
    case 1:
      printf("Add new customer\n");
      break;
    case 2:
      printf("Modify customer details\n");
      break;
case 3:
      printf("Process loan application\n");
      break;
    case 4:
      printf("Approve/Reject loans\n");
    case 5:
      printf("View assigned loan applications\n");
      break;
    case 6:
      printf("View customer Transactions\n");
      break;
    case 7:
      printf("Change password\n");
      break;
    case 8:
      printf("Logout\n");
      break;
    case 9:
      printf("Exit\n");
      break;
    default:
      printf("Invalid option\n");
  }

  return 0;
}
