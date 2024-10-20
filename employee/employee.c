#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "../global.c"

int main(int argc, char* argv[]){
  char* username = argv[0];
  char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");
   char LogOutPath[256];
    snprintf(LogOutPath, sizeof(LogOutPath), "%s%s", basePath, "/employee/logout.out");
  char AddCustomerPath[256];
    snprintf(AddCustomerPath, sizeof(AddCustomerPath), "%s%s", basePath, "/employee/addcustomer.out");
  
  char viewHistoryPath[256];
    snprintf(viewHistoryPath, sizeof(viewHistoryPath), "%s%s", basePath, "/employee/viewHistory.out");
  
  char updateUsernamePath[256];
    snprintf(updateUsernamePath, sizeof(updateUsernamePath), "%s%s", basePath, "/employee/updateUsername.out");
  
  char viewLoanApplicationsPath[256];
    snprintf(viewLoanApplicationsPath, sizeof(viewLoanApplicationsPath), "%s%s", basePath, "/employee/viewLoans.out");
  
  char loanApprovalPath[256];
    snprintf(loanApprovalPath, sizeof(loanApprovalPath), "%s%s", basePath, "/employee/loan.out");
  
  char changePasswordPath[256];
    snprintf(changePasswordPath, sizeof(changePasswordPath), "%s%s", basePath, "/employee/changepassword.out");

  printf("Welcome to the workspace dear employee\n");
  printf("Hello %s\n", username);
  printf("Select one of the following options to proceed further\n");
  printf("Add new customer -1\n");
  printf("Modify customer details -2\n");
  printf("Process loan applications -3\n");
  printf("Approve/reject loan applications -4\n");
  printf("View assigned loan applications -5\n");
  printf("View customer transactions -6\n");
  printf("Change password -7\n");
  printf("Logout -8\n");
  printf("Exit -9\n");

  int option;
  scanf("%d", &option);

  switch(option){
    case 1:
      printf("Add new customer\n");
      execvp(AddCustomerPath, argv);
      break;
    case 2:
      printf("Modify customer details\n");
      execvp(updateUsernamePath, argv);
      break;
    case 3:
      printf("Process loan application\n");
      break;
    case 4:
      printf("Approve/Reject loans\n");
      execvp(loanApprovalPath, argv);

    case 5:
      printf("View assigned loan applications\n");
      execvp(viewLoanApplicationsPath, argv);
      break;
    case 6:
      printf("View customer Transactions\n");
      execvp(viewHistoryPath, argv);
      break;
    case 7:
      printf("Change password\n");
      execvp(changePasswordPath, argv);
      break;
    case 8:
    printf("Logout\n");
            char *args[] = {username, NULL};
            execvp(LogOutPath, args);
            break;
    case 9:
      printf("Exit\n");
      execvp(ExitPath, NULL);
      break;
    default:
      printf("Invalid option\n");
  }

  return 0;
}
