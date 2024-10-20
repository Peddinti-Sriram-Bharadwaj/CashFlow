#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"

#define BUFFER_SIZE 256

void write_message(const char *message) {
  write(STDOUT_FILENO, message, strlen(message));
}

int main(int argc, char* argv[]) {
  char ManagerActionsPath[BUFFER_SIZE];
  snprintf(ManagerActionsPath, sizeof(ManagerActionsPath), "%s%s", basePath, "/Manager/manager.out");
  
  char ExitPath[BUFFER_SIZE];
  snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");

  char LogOutPath[BUFFER_SIZE];
  snprintf(LogOutPath, sizeof(LogOutPath), "%s%s", basePath, "/Manager/logout.out");

  char viewLoansPath[BUFFER_SIZE];
  snprintf(viewLoansPath, sizeof(viewLoansPath), "%s%s", basePath, "/Manager/viewLoans.out");

  char viewEmployeesPath[BUFFER_SIZE];
  snprintf(viewEmployeesPath, sizeof(viewEmployeesPath), "%s%s", basePath, "/Manager/viewEmployees.out");

  char assignLoanPath[BUFFER_SIZE];
  snprintf(assignLoanPath, sizeof(assignLoanPath), "%s%s", basePath, "/Manager/assignLoan.out");

  char changePasswordPath[BUFFER_SIZE];
  snprintf(changePasswordPath, sizeof(changePasswordPath), "%s%s", basePath, "/Manager/changePassword.out");

  char getFeedbackPath[BUFFER_SIZE];
  snprintf(getFeedbackPath, sizeof(getFeedbackPath), "%s%s", basePath, "/Manager/getFeedback.out");

  char* username = argv[0];

  write_message("========================================\n");
  write_message("Welcome to the admin dashboard\n");
  write_message("========================================\n");
  write_message("Hello ");
  write_message(username);
  write_message("\nPlease choose one of the below to proceed further\n");
  write_message("----------------------------------------\n");
  write_message("1. Activate/deactivate customer accounts\n");
  write_message("2. View Pending loan applications\n");
  write_message("3. View employees available for loan application processing\n");
  write_message("4. Assign loan application processes to employees\n");
  write_message("5. Review customer feedback\n");
  write_message("6. Change password\n");
  write_message("7. Logout\n");
  write_message("----------------------------------------\n");

  char buffer[BUFFER_SIZE];
  int option;
  ssize_t bytesRead = read(STDIN_FILENO, buffer, BUFFER_SIZE);
  if (bytesRead > 0) {
    buffer[bytesRead - 1] = '\0'; // Null-terminate the input
    option = atoi(buffer);
  } else {
    write_message("Failed to read input\n");
    return 1;
  }

  switch(option) {
    case 1: 
      write_message("activate/deactivate customer accounts\n");
      break;
    case 2:
      write_message("view pending loan applications\n");
      execvp(viewLoansPath, argv);
      break;
    case 3:
      write_message("view employees available for loan application processing\n");
      execvp(viewEmployeesPath, argv);
      break;
    case 4:
      write_message("assign loan application\n");
      execvp(assignLoanPath, argv);
      break;
    case 5:
      write_message("review customer feedback\n");
      execvp(getFeedbackPath, argv);
      break;
    case 6:
      write_message("change password\n");
      execvp(changePasswordPath, argv);
      break;
    case 7:
      write_message("Logout\n");
      execvp(LogOutPath, argv);
      break;
    default:
      write_message("invalid option\n");
      break;
  }
  return 0;
}
