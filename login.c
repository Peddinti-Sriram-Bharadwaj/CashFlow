#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(){
    printf("Welcome to 9217 Banking service\n");
    printf("Please select an option to proceed further\n");
    printf("login as:- ");
    printf("Customer:- 1\n");
    printf("Bank Employee:- 2\n");
    printf("Manager:- 3\n");
    printf("Administrator:- 4\n");

    int option;
    scanf("%d", &option);
    // case statements for different options, and exec functions to execute the respective programs
    switch(option){
        case 1:
            // exec customer program
            break;
        case 2:
            // exec bank employee program
            //execute the signin.out file in employee folder
            execl("./employee/signin.out", "./employee/signin.out", (char *)NULL);
            perror("execl failed"); // This line will only execute if execl fails
            break;
        case 3:
            // exec manager program
            break;
        case 4:
            // exec administrator program
            break;
        default:
            printf("Invalid option selected");
            break;
    }
}