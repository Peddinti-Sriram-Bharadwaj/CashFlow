#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(){

    // printf("Welcome to 9217 Banking services\n");
    //create a new account or login as existing user
    printf("Please select an option to proceed further\n");
    printf("Create a new account:- 1\n");
    printf("Login as existing user:- 2\n");
    int option;
    scanf("%d", &option);
    // case statements for different options, and exec functions to execute the respective programs
    switch(option){
        case 1:
            // exec create account program
            // use exec to execute the create.out file
            execl("./employee/create.out", "./employee/create.out", (char *)NULL);
            perror("execl failed"); // This line will only execute if execl fails

            break;
        case 2:
            // exec login program
            //use exec to execute the login.out file
            execl("./employee/login.out", "./employee/login.out", (char *)NULL);
            perror("execl failed"); // This line will only execute if execl fails
            
            break;
        default:
            printf("Invalid option selected\n");
            break;
    }
}