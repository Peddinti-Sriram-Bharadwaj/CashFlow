#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sodium.h> // Include libsodium header
#include "../global.c"



int main(int argc, char *argv[]) {
    char* username = argv[0];

    char ExitPath[256];
    snprintf(ExitPath, sizeof(ExitPath), "%s%s", basePath, "/welcome.out");

    char LogOutPath[256];
    snprintf(LogOutPath, sizeof(LogOutPath), "%s%s", basePath, "/customer/logout.out");

    char getBalancePath[256];
    snprintf(getBalancePath, sizeof(getBalancePath), "%s%s", basePath, "/customer/getBalance.out");

    char loanApplicationPath[256];
    snprintf(loanApplicationPath, sizeof(loanApplicationPath), "%s%s", basePath, "/customer/loanApplication.out");
    
    char depositMoneyPath[256];
    snprintf(depositMoneyPath, sizeof(depositMoneyPath), "%s%s", basePath, "/customer/depositMoney.out");

    char transferMoneyPath[256];
    snprintf(transferMoneyPath, sizeof(transferMoneyPath), "%s%s", basePath, "/customer/transferMoney.out");

    char withdrawMoneyPath[256];
    snprintf(withdrawMoneyPath, sizeof(withdrawMoneyPath), "%s%s", basePath, "/customer/withdrawMoney.out");

    char viewHistoryPath[256];
    snprintf(viewHistoryPath, sizeof(viewHistoryPath), "%s%s", basePath, "/customer/viewHistory.out");

    char changePasswordPath[256];
    snprintf(changePasswordPath, sizeof(changePasswordPath), "%s%s", basePath, "/customer/changePassword.out");

    char feedbackPath[256];
    snprintf(feedbackPath, sizeof(feedbackPath), "%s%s", basePath, "/customer/feedback.out");


    printf("welcome to Cashflow dear user\n");
    printf("Hello %s\n", username);
    printf("please select one of the below options to proceed further\n");
    printf("View account balance -1\n");
    printf("Deposit money -2\n");
    printf("Withdraw money -3\n");
    printf("Transfer funds -4\n");
    printf("Apply for a loan -5\n");
    printf("Change password -6\n");
    printf("Adding feedback -7\n");
    printf("View Transaction History -8\n");
    printf("Logout -9\n");
    printf("Exit -10\n");

    int option;
    scanf("%d", &option);

    switch(option){
        case 1:
            printf("View account balance\n");
            execvp(getBalancePath, argv);
            break;
        case 2:
            printf("Deposit money\n");
            execvp(depositMoneyPath, argv);
            break;
        case 3:
            printf("Withdraw money\n");
            execvp(withdrawMoneyPath, argv);
            break;
        case 4:
            printf("Transfer funds\n");
            execvp(transferMoneyPath, argv);
            break;
        case 5:
            printf("Apply for a loan\n");
            execvp(loanApplicationPath, argv);
            break;
        case 6:
            printf("Change password\n");
            execvp(changePasswordPath, argv);
            break;
        case 7:
            printf("Adding feedback\n");
            execvp(feedbackPath, argv);
            break;
        case 8:
            printf("View Transaction History\n");
            execvp(viewHistoryPath, argv);
            break;
        case 9:
            printf("Logout\n");
            char *args[] = {username, NULL};
            execvp(LogOutPath,args );

            break;
        case 10:
            printf("Exit\n");
            execvp(ExitPath, NULL);
            break;
        default:
            printf("Invalid option\n");
    }
}
