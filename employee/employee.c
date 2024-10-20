#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "../global.c"

void print_message(const char *message) {
  write(STDOUT_FILENO, message, strlen(message));
}

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

  print_message("Welcome to the workspace dear employee\n");
  print_message("Hello ");
  print_message(username);
  print_message("\nSelect one of the following options to proceed further\n");
  print_message("Add new customer -1\n");
  print_message("Modify customer details -2\n");
  print_message("Process loan applications -3\n");
  print_message("Approve/reject loan applications -4\n");
  print_message("View assigned loan applications -5\n");
  print_message("View customer transactions -6\n");
  print_message("Change password -7\n");
  print_message("Logout -8\n");
  print_message("Exit -9\n");

  char buffer[4];
  int option;
  read(STDIN_FILENO, buffer, sizeof(buffer));
  option = atoi(buffer);

  switch(option){
    case 1:
      print_message("Add new customer\n");
      execvp(AddCustomerPath, argv);
      break;
    case 2:
      print_message("Modify customer details\n");
      execvp(updateUsernamePath, argv);
      break;
    case 3:
      print_message("Process loan application\n");
      break;
    case 4:
      print_message("Approve/Reject loans\n");
      execvp(loanApprovalPath, argv);
      break;
    case 5:
      print_message("View assigned loan applications\n");
      execvp(viewLoanApplicationsPath, argv);
      break;
    case 6:
      print_message("View customer Transactions\n");
      execvp(viewHistoryPath, argv);
      break;
    case 7:
      print_message("Change password\n");
      execvp(changePasswordPath, argv);
      break;
    case 8:
      print_message("Logout\n");
      char *args[] = {username, NULL};
      execvp(LogOutPath, args);
      break;
    case 9:
      print_message("Exit\n");
      execvp(ExitPath, NULL);
      break;
    default:
      print_message("Invalid option\n");
  }

  return 0;
}
