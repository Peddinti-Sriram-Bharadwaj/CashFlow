#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
    printf("welcome to Cashflow dear user\n");
    printf("please select one of the below options to proceed further");
    printf("View account balance -1");
    printf("Deposit money -2");
    printf("Withdraw money -3");
    printf("Transfer funds -4");
    printf("Apply for a loan -5");
    printf("Change password -6");
    printf("Adding feedback -7");
    printf("View Transaction History -8");
    printf("Logout -9");
    printf("Exit -10");

    int option;
    scanf("%d", &option);

    switch(option){
        case 1:
            printf("View account balance\n");
            break;
        case 2:
            printf("Deposit money\n");
            break;
        case 3:
            printf("Withdraw money\n");
            break;
        case 4:
            printf("Transfer funds\n");
            break;
        case 5:
            printf("Apply for a loan\n");
            break;
        case 6:
            printf("Change password\n");
            break;
        case 7:
            printf("Adding feedback\n");
            break;
        case 8:
            printf("View Transaction History\n");
            break;
        case 9:
            printf("Logout\n");
            break;
        case 10:
            printf("Exit\n");
            break;
        default:
            printf("Invalid option\n");
    }
}
