#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"
#include <string.h>

#define BUFFER_SIZE 256

void print_message(const char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

int main(int argc, char *argv[]) {
    char* username = argv[0];

    char ExitPath[BUFFER_SIZE];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");

    char LogOutPath[BUFFER_SIZE];
    snprintf(LogOutPath, sizeof(LogOutPath), "%s%s", basePath, "/customer/logout.out");

    char getBalancePath[BUFFER_SIZE];
    snprintf(getBalancePath, sizeof(getBalancePath), "%s%s", basePath, "/customer/getBalance.out");

    char loanApplicationPath[BUFFER_SIZE];
    snprintf(loanApplicationPath, sizeof(loanApplicationPath), "%s%s", basePath, "/customer/loanApplication.out");
    
    char depositMoneyPath[BUFFER_SIZE];
    snprintf(depositMoneyPath, sizeof(depositMoneyPath), "%s%s", basePath, "/customer/depositMoney.out");

    char transferMoneyPath[BUFFER_SIZE];
    snprintf(transferMoneyPath, sizeof(transferMoneyPath), "%s%s", basePath, "/customer/transferMoney.out");

    char withdrawMoneyPath[BUFFER_SIZE];
    snprintf(withdrawMoneyPath, sizeof(withdrawMoneyPath), "%s%s", basePath, "/customer/withdrawMoney.out");

    char viewHistoryPath[BUFFER_SIZE];
    snprintf(viewHistoryPath, sizeof(viewHistoryPath), "%s%s", basePath, "/customer/viewHistory.out");

    char changePasswordPath[BUFFER_SIZE];
    snprintf(changePasswordPath, sizeof(changePasswordPath), "%s%s", basePath, "/customer/changePassword.out");

    char feedbackPath[BUFFER_SIZE];
    snprintf(feedbackPath, sizeof(feedbackPath), "%s%s", basePath, "/customer/feedback.out");

    print_message("========================================\n");
    print_message("Welcome to Cashflow dear user\n");
    print_message("Hello ");
    print_message(username);
    print_message("\nPlease select one of the below options to proceed further\n");
    print_message("========================================\n");
    print_message("1. View account balance\n");
    print_message("2. Deposit money\n");
    print_message("3. Withdraw money\n");
    print_message("4. Transfer funds\n");
    print_message("5. Apply for a loan\n");
    print_message("6. Change password\n");
    print_message("7. Adding feedback\n");
    print_message("8. View Transaction History\n");
    print_message("9. Logout\n");
    print_message("========================================\n");

    char buffer[BUFFER_SIZE];
    int bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);
    if (bytes_read <= 0) {
        print_message("Failed to read input\n");
        return 1;
    }

    int option = atoi(buffer);

    switch(option){
        case 1:
            print_message("View account balance\n");
            execvp(getBalancePath, argv);
            break;
        case 2:
            print_message("Deposit money\n");
            execvp(depositMoneyPath, argv);
            break;
        case 3:
            print_message("Withdraw money\n");
            execvp(withdrawMoneyPath, argv);
            break;
        case 4:
            print_message("Transfer funds\n");
            execvp(transferMoneyPath, argv);
            break;
        case 5:
            print_message("Apply for a loan\n");
            execvp(loanApplicationPath, argv);
            break;
        case 6:
            print_message("Change password\n");
            execvp(changePasswordPath, argv);
            break;
        case 7:
            print_message("Adding feedback\n");
            execvp(feedbackPath, argv);
            break;
        case 8:
            print_message("View Transaction History\n");
            execvp(viewHistoryPath, argv);
            break;
        case 9:
            print_message("Logout\n");
            char *args[] = {username, NULL};
            execvp(LogOutPath, args);
            break;
        default:
            print_message("Invalid option\n");
    }

    return 0;
}
