#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
    printf("welcome to Cashflow dear user\n");
    printf("please login to proceed further\n");
    printf("Enter the username\n");
    char username[50];
    scanf("%s", username);
    printf("Enter the password\n");
    char password[50];
    scanf("%s", password);
    printf("login successful\n");
}