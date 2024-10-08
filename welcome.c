#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(){
    printf("Welcome to CashFlow banking services.\n");
    printf("PLease choose one of the following options to continue.\n");
    printf("Admin -1\n");
    printf("Manager -2\n");
    printf("Employee -3\n");
    printf("Customer -4\n");
    
    int option;
    scanf("%d", &option);
    switch(option){
        case 1:
            printf("Admin\n");
            break;
        case 2:
            printf("Manager\n");
            break;
        case 3:
            printf("Employee\n");
            break;
        case 4:
            printf("Customer\n");
            break;
        default:
            printf("Invalid option\n");
    }

}
