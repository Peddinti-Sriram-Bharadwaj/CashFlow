#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
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

  print_message("********************************************\n");
  print_message("* Welcome to the workspace dear employee   *\n");
  print_message("********************************************\n");
  print_message("Hello ");
  print_message(username);
  print_message("\n");
  print_message("********************************************\n");
  print_message("* Select one of the following options to   *\n");
  print_message("* proceed further                          *\n");
  print_message("********************************************\n");
  print_message("* 1. Add new customer                      *\n");
  print_message("* 2. Modify customer details               *\n");
  print_message("* 4. Approve/reject loan applications      *\n");
  print_message("* 5. View assigned loan applications       *\n");
  print_message("* 6. View customer transactions            *\n");
  print_message("* 7. Change password                       *\n");
  print_message("* 8. Logout                                *\n");
  print_message("********************************************\n");

  char buffer[4];
  int option;
  read(STDIN_FILENO, buffer, sizeof(buffer));
  option = atoi(buffer);

  switch(option){
    case 1:
      print_message("********************************************\n");
      print_message("* Add new customer                         *\n");
      print_message("********************************************\n");
      execvp(AddCustomerPath, argv);
      break;
    case 2:
      print_message("********************************************\n");
      print_message("* Modify customer details                  *\n");
      print_message("********************************************\n");
      execvp(updateUsernamePath, argv);
      break;
    case 4:
      print_message("********************************************\n");
      print_message("* Approve/Reject loans                     *\n");
      print_message("********************************************\n");
      execvp(loanApprovalPath, argv);
      break;
    case 5:
      print_message("********************************************\n");
      print_message("* View assigned loan applications          *\n");
      print_message("********************************************\n");
      execvp(viewLoanApplicationsPath, argv);
      break;
    case 6:
      print_message("********************************************\n");
      print_message("* View customer Transactions               *\n");
      print_message("********************************************\n");
      execvp(viewHistoryPath, argv);
      break;
    case 7:
      print_message("********************************************\n");
      print_message("* Change password                          *\n");
      print_message("********************************************\n");
      execvp(changePasswordPath, argv);
      break;
    case 8:
      print_message("********************************************\n");
      print_message("* Logout                                   *\n");
      print_message("********************************************\n");
      char *args[] = {username, NULL};
      execvp(LogOutPath, args);
      break;
    default:
      print_message("********************************************\n");
      print_message("* Invalid option                           *\n");
      print_message("********************************************\n");
  }

  return 0;
}
