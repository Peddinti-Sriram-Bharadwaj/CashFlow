#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

struct customer{
  char username[50];
  char password[50];
  char email[50];
  char phone[10];
  char address[100];
  char dob[10];
  char  account_number[10]; 
  char account_type[10];
  char account_balance[10];
};

int main(){
  printf("please enter the following details\n");
  char username[50];
  char password[50];
  printf("enter username\n");
  scanf("%s", username);
  printf("Enter the password\n");
  scanf("%s",password);
  printf("enter email\n");
  char email[50];
  scanf("%s", email);
  printf("enter phone number\n");
  char phone[10];
  scanf("%s", phone);
  printf("enter address\n");
  char address[100];
  scanf("%s", address);
  printf("enter date of birth\n");
  char dob[10];
  scanf("%s", dob);
  printf("enter account number\n");
  char account_number[10];
  scanf("%s", account_number);
  printf("enter account type\n");
  char account_type[10];
  scanf("%s", account_type);
  printf("enter account balance\n");
  char account_balance[10];
  scanf("%s", account_balance);
  struct customer c;
  strcpy(c.username, username);
  strcpy(c.password, password);
  strcpy(c.email, email);
  strcpy(c.phone, phone);
  strcpy(c.address, address);
  strcpy(c.dob, dob);
  strcpy(c.account_number, account_number);
  strcpy(c.account_type, account_type);
  strcpy(c.account_balance, account_balance);
  int fd = open("customer.txt", O_CREAT | O_WRONLY, 0777);
  write(fd, &c, sizeof(c));
  close(fd);
  printf("account created successfully");
}
