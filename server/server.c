#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

#define SOCKET_PATH "/tmp/server"
#define BUFFER_SIZE 256

pthread_mutex_t customer_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t admin_file_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex to synchronize file access for admins
pthread_mutex_t employee_file_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex to synchronize file access for employees
pthread_mutex_t manager_file_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex to synchronize file access for managers
pthread_mutex_t loan_application_file_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex to synchronize file access for loan applications
// Mutex for file operations
pthread_mutex_t feedback_file_mutex = PTHREAD_MUTEX_INITIALIZER;



struct Customer {
    char username[20];
    int balance;
};

struct Admin {
    char username[20];
};

#define MAX_EMPLOYEES 5

struct Employee {
    char username[20];                   // Field for the employee's username
    char processing_usernames[5][20]; // Array to hold 5 usernames of customers whose loans are being processed
};


struct Manager {
    char username[20];
};

struct Operation {
    char operation[20];
    union {
        struct Customer customer;
        struct Admin admin;
        struct Employee employee;
        struct Manager manager;
    } data;
};

struct BalanceRequest {
    struct Customer customer;
    int client_sockfd; // Client socket file descriptor
};

struct LoanRequest {
    char username[20];
    int client_sockfd; // Client socket file descriptor
};

struct LoanApplication {
    char username[20];
    int amount;
    char assigned_employee[20]; // Initially set to "None"
};

// Struct for deposit request
struct DepositRequest {
    char username[20];     // Username of the customer
    int client_sockfd;     // Client socket file descriptor
};

// Struct for deposit application
struct DepositApplication {
    char username[20];     // Username of the customer
    int amount;            // Amount to be deposited
};

// Struct for transfer request
struct TransferRequest {
    char username[20];     // Username of the customer
    int client_sockfd;     // Client socket file descriptor
};

// Struct for transfer application
struct TransferApplication {
    char from_username[20];  // Username of the sender
    char to_username[20];    // Username of the recipient
    int amount;              // Amount to be transferred
};

struct WithdrawalRequest {
    char username[20];  // To store the customer's username
    int client_sockfd;  // To store the client socket file descriptor
};

// Transaction structure for each transaction (Deposit, Withdrawal, Transfer)
struct Transaction {
    char type[20];      // Type of transaction (Deposit, Withdrawal, Transfer)
    int amount;         // Amount for the transaction
    char date[20];      // Date of the transaction
    char from_username[20];  // For Transfers: Username of the sender
    char to_username[20];    // For Transfers: Username of the recipient
};

// Passbook structure containing transaction history and username
struct Passbook {
    char username[20];           // Username of the customer
    int num_transactions;        // Number of transactions in the passbook
    struct Transaction transactions[100];  // Array to store the transactions
};

struct PassbookRequest {
    char username[20];   // Username of the customer whose passbook is requested
    int client_sockfd;   // Socket file descriptor to send the passbook to the client
};

struct FeedbackRequest {
    char username[20];
    int client_sockfd; // Client socket file descriptor
};

struct Feedback {
    char username[20];
    char feedback[101]; // Feedback field with max length of 100 chars + null terminator
};

struct EmployeeRequest {
    char username[20];
    int client_sockfd; // Client socket file descriptor
};

struct EmployeeLoanRequest {
    char employee_username[20]; // Employee's username
    int client_sockfd;          // Client socket descriptor
};

struct LoanApprovalRequest {
    char username[20];  // Customer's username
    int amount;         // Amount of the loan
};


// Function to handle adding a customer
void *addcustomer(void *arg) {
    struct Customer *customer = (struct Customer *)arg;

    // Lock the mutex before accessing the customer file
    pthread_mutex_lock(&customer_file_mutex);

    // Open the customers file for appending
    FILE *customer_file = fopen("customers.txt", "ab");
    if (customer_file == NULL) {
        perror("fopen");
        pthread_mutex_unlock(&customer_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    // Write the customer data to the file
    if (fwrite(customer, sizeof(struct Customer), 1, customer_file) != 1) {
        perror("fwrite");
        fclose(customer_file);
        pthread_mutex_unlock(&customer_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    fclose(customer_file);

    // Now add the empty passbook to the transactions file
    FILE *passbook_file = fopen("transactions.txt", "ab");
    if (passbook_file == NULL) {
        perror("fopen");
        pthread_mutex_unlock(&customer_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    // Initialize the empty passbook
    struct Passbook passbook;
    strcpy(passbook.username, customer->username);  // Set the username from the customer struct
    passbook.num_transactions = 0;  // No transactions yet

    // Write the empty passbook to the file
    if (fwrite(&passbook, sizeof(struct Passbook), 1, passbook_file) != 1) {
        perror("fwrite");
        fclose(passbook_file);
        pthread_mutex_unlock(&customer_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    fclose(passbook_file);

    // Unlock the mutex after the file operation is complete
    pthread_mutex_unlock(&customer_file_mutex);

    pthread_exit(NULL);
}

void *addadmin(void *arg) {
    struct Admin *admin = (struct Admin *)arg;

    // Lock the mutex before accessing the file
    pthread_mutex_lock(&admin_file_mutex);

    FILE *file = fopen("admins.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        pthread_mutex_unlock(&admin_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    if (fwrite(admin, sizeof(struct Admin), 1, file) != 1) {
        perror("fwrite");
        fclose(file);
        pthread_mutex_unlock(&admin_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    fclose(file);

    // Unlock the mutex after the file operation is complete
    pthread_mutex_unlock(&admin_file_mutex);

    pthread_exit(NULL);
}

// Function to handle adding an employee
void *addemployee(void *arg) {
    struct Employee *employee = (struct Employee *)arg;

    // Initialize processing usernames to "None"
    for (int i = 0; i < MAX_EMPLOYEES; i++) {
        strncpy(employee->processing_usernames[i], "None", sizeof(employee->processing_usernames[i]) - 1);
        employee->processing_usernames[i][sizeof(employee->processing_usernames[i]) - 1] = '\0'; // Null-terminate
    }

    // Lock the mutex before accessing the file
    pthread_mutex_lock(&employee_file_mutex);

    FILE *file = fopen("employees.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        pthread_mutex_unlock(&employee_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    if (fwrite(employee, sizeof(struct Employee), 1, file) != 1) {
        perror("fwrite");
        fclose(file);
        pthread_mutex_unlock(&employee_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    fclose(file);

    // Unlock the mutex after the file operation is complete
    pthread_mutex_unlock(&employee_file_mutex);

    pthread_exit(NULL);
}

// Function to handle adding a manager
void *addmanager(void *arg) {
    struct Manager *manager = (struct Manager *)arg;

    // Lock the mutex before accessing the file
    pthread_mutex_lock(&manager_file_mutex);

    FILE *file = fopen("managers.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        pthread_mutex_unlock(&manager_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    if (fwrite(manager, sizeof(struct Manager), 1, file) != 1) {
        perror("fwrite");
        fclose(file);
        pthread_mutex_unlock(&manager_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    fclose(file);

    // Unlock the mutex after the file operation is complete
    pthread_mutex_unlock(&manager_file_mutex);

    pthread_exit(NULL);
}


void *getbalance(void *arg) {
    struct BalanceRequest *request = (struct BalanceRequest *)arg;
    int client_sockfd = request->client_sockfd;

    // Validate socket descriptor before proceeding
    printf("Client socket descriptor: %d\n", client_sockfd);
    if (client_sockfd < 0) {
        fprintf(stderr, "Invalid socket descriptor\n");
        free(request); // Free memory before exiting
        pthread_exit(NULL);
    }

    // Log the request details
    printf("Getting balance for customer: %s\n", request->customer.username);

    // Lock the mutex for safe file access
    pthread_mutex_lock(&customer_file_mutex);
    FILE *file = fopen("customers.txt", "rb");
    if (file == NULL) {
        perror("Failed to open customers.txt");
        pthread_mutex_unlock(&customer_file_mutex); // Unlock before exiting
        free(request); // Free memory before exiting
        pthread_exit(NULL);
    }

    struct Customer temp_customer;
    int found = 0;

    // Search for the customer in the file
    while (fread(&temp_customer, sizeof(struct Customer), 1, file) == 1) {
        if (strcmp(temp_customer.username, request->customer.username) == 0) {
            printf("Found customer: %s with balance: %d\n", temp_customer.username, temp_customer.balance);
            found = 1;

            // Attempt to send the balance
            printf("Attempting to send balance: %d to socket: %d\n", temp_customer.balance, client_sockfd);
            ssize_t num_bytes_sent = send(client_sockfd, &temp_customer.balance, sizeof(temp_customer.balance), 0);
            printf("%d\n", (int)num_bytes_sent);
            if (num_bytes_sent == -1) {
                perror("send");
                // Handle the error appropriately here
            }
            break;
        }
    }

    // Check if customer was found
    if (!found) {
        int not_found = -1; // Indicate user not found
        printf("Customer not found, sending -1 to client.\n");
        ssize_t num_bytes_sent = send(client_sockfd, &not_found, sizeof(not_found), 0);
        printf("%d\n", (int)num_bytes_sent);
        if (num_bytes_sent == -1) {
            perror("send");
            // Handle the error appropriately here
        }
    }

    fclose(file); // Close the file
    pthread_mutex_unlock(&customer_file_mutex); // Unlock after file access

    // Free allocated memory for the request
    free(request); // Make sure to free the allocated memory for request

    close(client_sockfd); // Close the client socket
    pthread_exit(NULL);
}




void *handle_loan_application(void *arg) {
    struct LoanRequest *request = (struct LoanRequest *)arg;
    int client_sockfd = request->client_sockfd;

    // Step 1: Send "amount" to the client
    char send_message[] = "amount";
    ssize_t num_bytes_sent = send(client_sockfd, send_message, sizeof(send_message), 0);
    if (num_bytes_sent == -1) {
        perror("send");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 2: Receive the loan amount from the client
    int loan_amount;
    ssize_t num_bytes_received = recv(client_sockfd, &loan_amount, sizeof(loan_amount), 0);
    if (num_bytes_received == -1) {
        perror("recv");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 3: Create the LoanApplication structure and assign values
    struct LoanApplication loan_application;
    strncpy(loan_application.username, request->username, sizeof(loan_application.username) - 1);
    loan_application.username[sizeof(loan_application.username) - 1] = '\0'; // Null-terminate
    loan_application.amount = loan_amount;
    strncpy(loan_application.assigned_employee, "None", sizeof(loan_application.assigned_employee) - 1);
    loan_application.assigned_employee[sizeof(loan_application.assigned_employee) - 1] = '\0'; // Null-terminate

    // Step 4: Lock the mutex before accessing the loan applications file
    pthread_mutex_lock(&loan_application_file_mutex);

    // Write the structure to the loanApplications.txt file
    FILE *file = fopen("loanApplications.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        pthread_mutex_unlock(&loan_application_file_mutex);  // Unlock before exiting
        close(client_sockfd);
        pthread_exit(NULL);
    }

    if (fwrite(&loan_application, sizeof(struct LoanApplication), 1, file) != 1) {
        perror("fwrite");
        fclose(file);

        // Send failure response to the client
        int failure_message = -1;
        send(client_sockfd, &failure_message, sizeof(failure_message), 0);

        pthread_mutex_unlock(&loan_application_file_mutex);  // Unlock before exiting
        close(client_sockfd);
        pthread_exit(NULL);
    }

    fclose(file);

    // Step 5: Send success message to the client
    int success_message = 1;
    ssize_t success_bytes_sent = send(client_sockfd, &success_message, sizeof(success_message), 0);
    if (success_bytes_sent == -1) {
        perror("send");
    }

    // Step 6: Unlock the mutex after the file operation is complete
    pthread_mutex_unlock(&loan_application_file_mutex);

    // Step 7: Clean up and exit the thread
    close(client_sockfd);
    pthread_exit(NULL);
}

void *update_username(void *arg) {
    struct PassbookRequest *request = (struct PassbookRequest *)arg;
    int client_sockfd = request->client_sockfd;
    char *current_username = request->username;

    // Lock the mutex before accessing the file
    pthread_mutex_lock(&customer_file_mutex);

    FILE *customer_file = fopen("customers.txt", "rb+");
    if (customer_file == NULL) {
        perror("fopen customers.txt");
        pthread_mutex_unlock(&customer_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    struct Customer customer;
    int user_found = 0;

    // Check if the current username exists in customers.txt
    while (fread(&customer, sizeof(struct Customer), 1, customer_file) == 1) {
        if (strcmp(customer.username, current_username) == 0) {
            user_found = 1;
            break;
        }
    }

    // If user not found, unlock and exit
    if (!user_found) {
        fclose(customer_file);
        pthread_mutex_unlock(&customer_file_mutex);
        pthread_exit(NULL);
    }

    // Send the message "newname" to the client, asking for the new username
    char message[] = "newname";
    printf("%s\n", message);   
    ssize_t num_bytes_sent = send(client_sockfd, message, sizeof(message), 0);
    if (num_bytes_sent == -1) {
        perror("send newname request");
        fclose(customer_file);
        pthread_mutex_unlock(&customer_file_mutex);
        pthread_exit(NULL);
    }

    // Receive the new username from the client
    char new_username[20];
    ssize_t num_bytes_received = recv(client_sockfd, new_username, sizeof(new_username), 0);
    if (num_bytes_received == -1) {
        perror("recv new_username");
        fclose(customer_file);
        pthread_mutex_unlock(&customer_file_mutex);
        pthread_exit(NULL);
    }
    new_username[num_bytes_received] = '\0';  // Null-terminate the new username

    // Check if the new username is already taken
    rewind(customer_file);  // Reset file pointer to the beginning
    while (fread(&customer, sizeof(struct Customer), 1, customer_file) == 1) {
        if (strcmp(customer.username, new_username) == 0) {
            // Send an error response to the client: new name already exists
            int response = -1;
            send(client_sockfd, &response, sizeof(response), 0);
            fclose(customer_file);
            pthread_mutex_unlock(&customer_file_mutex);
            pthread_exit(NULL);
        }
    }

    // New username is available, update the passbook and customer files

    // Rewind the file again to update the current user's name
    rewind(customer_file);
    while (fread(&customer, sizeof(struct Customer), 1, customer_file) == 1) {
        if (strcmp(customer.username, current_username) == 0) {
            // Update the username in customers.txt
            fseek(customer_file, -sizeof(struct Customer), SEEK_CUR);  // Move the file pointer to the current record
            strncpy(customer.username, new_username, sizeof(customer.username) - 1);
            customer.username[sizeof(customer.username) - 1] = '\0';  // Null-terminate
            fwrite(&customer, sizeof(struct Customer), 1, customer_file);
            fflush(customer_file);  // Ensure changes are written to the file
            break;
        }
    }

    fclose(customer_file);

    // Open transactions.txt to update the passbook and transactions
    FILE *transaction_file = fopen("transactions.txt", "rb+");
    if (transaction_file == NULL) {
        perror("fopen transactions.txt");
        pthread_mutex_unlock(&customer_file_mutex);
        pthread_exit(NULL);
    }

    struct Passbook passbook;
    // Iterate through the passbooks to update the username and target transactions
    while (fread(&passbook, sizeof(struct Passbook), 1, transaction_file) == 1) {
        if (strcmp(passbook.username, current_username) == 0) {
            // Update the username in the passbook
            fseek(transaction_file, -sizeof(struct Passbook), SEEK_CUR);
            strncpy(passbook.username, new_username, sizeof(passbook.username) - 1);
            passbook.username[sizeof(passbook.username) - 1] = '\0';
            fwrite(&passbook, sizeof(struct Passbook), 1, transaction_file);
            fflush(transaction_file);
        }

        // Update all transactions with target 'current_username' to 'new_username'
        for (int i = 0; i < passbook.num_transactions; i++) {
            if (strcmp(passbook.transactions[i].from_username, current_username) == 0) {
                strncpy(passbook.transactions[i].from_username, new_username, sizeof(passbook.transactions[i].from_username) - 1);
                passbook.transactions[i].from_username[sizeof(passbook.transactions[i].from_username) - 1] = '\0';
            }
            if (strcmp(passbook.transactions[i].to_username, current_username) == 0) {
                strncpy(passbook.transactions[i].to_username, new_username, sizeof(passbook.transactions[i].to_username) - 1);
                passbook.transactions[i].to_username[sizeof(passbook.transactions[i].to_username) - 1] = '\0';
            }
        }
    }

    fclose(transaction_file);

    // Unlock the mutex after completing the file operations
    pthread_mutex_unlock(&customer_file_mutex);

        // Open loanApplications.txt to update the username in loan applications
    FILE *loan_file = fopen("loanApplications.txt", "rb+");
    if (loan_file == NULL) {
        perror("fopen loanApplications.txt");
        pthread_mutex_unlock(&customer_file_mutex);
        pthread_exit(NULL);
    }

    struct LoanApplication loan_app;
    // Iterate through the loan applications to update the username
    while (fread(&loan_app, sizeof(struct LoanApplication), 1, loan_file) == 1) {
        if (strcmp(loan_app.username, current_username) == 0) {
            // Update the username in loanApplications.txt
            fseek(loan_file, -sizeof(struct LoanApplication), SEEK_CUR);  // Move the file pointer to the current record
            strncpy(loan_app.username, new_username, sizeof(loan_app.username) - 1);
            loan_app.username[sizeof(loan_app.username) - 1] = '\0';  // Null-terminate
            fwrite(&loan_app, sizeof(struct LoanApplication), 1, loan_file);
            fflush(loan_file);  // Ensure changes are written to the file
        }
    }

    fclose(loan_file);


    // Send a success message to the client
    int success = 1;
    send(client_sockfd, &success, sizeof(success), 0);

    pthread_exit(NULL);
}


void *handle_deposit_application(void *arg) {
    struct DepositRequest *request = (struct DepositRequest *)arg;
    int client_sockfd = request->client_sockfd;

    // Check for valid socket descriptor
    if (client_sockfd < 0) {
        fprintf(stderr, "Invalid socket descriptor\n");
        pthread_exit(NULL);
    }

    // Step 1: Send "amount" prompt to the client
    char send_message[] = "amount";
    ssize_t num_bytes_sent = send(client_sockfd, send_message, sizeof(send_message), 0);
    if (num_bytes_sent == -1) {
        perror("send");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 2: Receive the deposit amount from the client
    int deposit_amount;
    ssize_t num_bytes_received = recv(client_sockfd, &deposit_amount, sizeof(deposit_amount), 0);
    if (num_bytes_received <= 0) { // Handle connection closed or error
        perror("recv");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 3: Read the customers file and update the balance for the specified customer
    FILE *file = fopen("customers.txt", "r+b");
    if (file == NULL) {
        perror("fopen");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    struct Customer temp_customer;
    int found = 0;

    // Lock the mutex to ensure exclusive access to the customer file
    pthread_mutex_lock(&customer_file_mutex);

    // Search for the customer and update their balance
    while (fread(&temp_customer, sizeof(struct Customer), 1, file) == 1) {
        if (strcmp(temp_customer.username, request->username) == 0) {
            found = 1;
            temp_customer.balance += deposit_amount;

            // Seek back and update the record
            fseek(file, -sizeof(struct Customer), SEEK_CUR);
            fwrite(&temp_customer, sizeof(struct Customer), 1, file);

            // Update the customer's passbook with the deposit transaction
            FILE *transaction_file = fopen("transactions.txt", "r+b");
            if (transaction_file == NULL) {
                perror("fopen transactions.txt");
                fclose(file);
                pthread_mutex_unlock(&customer_file_mutex);
                close(client_sockfd);
                pthread_exit(NULL);
            }

            struct Passbook passbook;
            int passbook_found = 0;

            // Search for the passbook of the customer
            while (fread(&passbook, sizeof(struct Passbook), 1, transaction_file) == 1) {
                if (strcmp(passbook.username, request->username) == 0) {
                    passbook_found = 1;

                    // Create the deposit transaction
                    struct Transaction deposit_transaction;
                    strcpy(deposit_transaction.type, "Deposit");
                    deposit_transaction.amount = deposit_amount;
                    strcpy(deposit_transaction.date, "2024-10-19"); // Set the actual date
                    strcpy(deposit_transaction.from_username, request->username);
                    strcpy(deposit_transaction.to_username, request->username);

                    // Add the deposit transaction to the passbook
                    passbook.transactions[passbook.num_transactions++] = deposit_transaction;

                    // Seek back and update the passbook record
                    fseek(transaction_file, -sizeof(struct Passbook), SEEK_CUR);
                    fwrite(&passbook, sizeof(struct Passbook), 1, transaction_file);
                    break;
                }
            }

            fclose(transaction_file);
            break;
        }
    }

    // Unlock the mutex after updating the customer file
    pthread_mutex_unlock(&customer_file_mutex);

    fclose(file);

    // Step 4: Send success or failure message to the client
    int response = found ? 1 : -1;
    num_bytes_sent = send(client_sockfd, &response, sizeof(response), 0);
    if (num_bytes_sent == -1) {
        perror("send");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 5: Clean up and exit the thread
    close(client_sockfd);
    pthread_exit(NULL);
}

void *handle_transfer_money(void *arg) {
    struct TransferRequest *request = (struct TransferRequest *)arg;
    int client_sockfd = request->client_sockfd;
    char *username = request->username;

    // Step 1: Send "amount" prompt to the client
    char send_message[] = "amount";
    ssize_t num_bytes_sent = send(client_sockfd, send_message, sizeof(send_message), 0);
    if (num_bytes_sent == -1) {
        perror("send");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 2: Receive the transfer amount from the client
    int transfer_amount;
    ssize_t num_bytes_received = recv(client_sockfd, &transfer_amount, sizeof(transfer_amount), 0);
    if (num_bytes_received == -1) {
        perror("recv");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 3: Send "to_recipient" prompt to the client
    char recipient_message[] = "to_recipient";
    num_bytes_sent = send(client_sockfd, recipient_message, sizeof(recipient_message), 0);
    if (num_bytes_sent == -1) {
        perror("send");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 4: Receive the recipient's username from the client
    char to_username[20];
    num_bytes_received = recv(client_sockfd, to_username, sizeof(to_username), 0);
    if (num_bytes_received == -1) {
        perror("recv");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 5: Read the customers file and check if the recipient exists
    FILE *file = fopen("customers.txt", "r+b");
    if (file == NULL) {
        perror("fopen");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    struct Customer temp_customer;
    int sender_found = 0, receiver_found = 0;

    // Lock the mutex to ensure exclusive access to the customer file
    pthread_mutex_lock(&customer_file_mutex);

    // Search for the recipient in the customers file first
    while (fread(&temp_customer, sizeof(struct Customer), 1, file) == 1) {
        if (strcmp(temp_customer.username, to_username) == 0) {
            receiver_found = 1; // Recipient found
            break; // No need to keep searching if we find the recipient
        }
    }

    // If the recipient is found, search for the sender and process the transfer
    if (receiver_found) {
        // Go back to the beginning of the file to search for the sender
        fseek(file, 0, SEEK_SET);

        while (fread(&temp_customer, sizeof(struct Customer), 1, file) == 1) {
            if (strcmp(temp_customer.username, username) == 0) {
                sender_found = 1; // Sender found
                if (temp_customer.balance >= transfer_amount) {
                    temp_customer.balance -= transfer_amount; // Deduct transfer amount from sender
                    // Seek back and update the sender record
                    fseek(file, -sizeof(struct Customer), SEEK_CUR);
                    fwrite(&temp_customer, sizeof(struct Customer), 1, file);

                    // Now update the recipient's balance
                    while (fread(&temp_customer, sizeof(struct Customer), 1, file) == 1) {
                        if (strcmp(temp_customer.username, to_username) == 0) {
                            temp_customer.balance += transfer_amount; // Add transfer amount to receiver
                            // Seek back and update the receiver record
                            fseek(file, -sizeof(struct Customer), SEEK_CUR);
                            fwrite(&temp_customer, sizeof(struct Customer), 1, file);
                            break;
                        }
                    }

                    // Step 8: Update passbook for both sender and receiver
                    FILE *transaction_file = fopen("transactions.txt", "r+b");
                    if (transaction_file == NULL) {
                        perror("fopen transactions.txt");
                        pthread_mutex_unlock(&customer_file_mutex); // Unlock before exiting
                        fclose(file);
                        close(client_sockfd);
                        pthread_exit(NULL);
                    }

                    struct Passbook passbook;
                    // Update sender's passbook
                    while (fread(&passbook, sizeof(struct Passbook), 1, transaction_file) == 1) {
                        if (strcmp(passbook.username, username) == 0) {
                            // Create a new transaction record for sender
                            struct Transaction sender_transaction;
                            strcpy(sender_transaction.type, "Transfer Out");
                            sender_transaction.amount = transfer_amount;
                            strcpy(sender_transaction.date, "2024-10-19"); // Use actual date here
                            strcpy(sender_transaction.from_username, username);
                            strcpy(sender_transaction.to_username, to_username);

                            // Add the transaction to the sender's passbook
                            passbook.transactions[passbook.num_transactions++] = sender_transaction;

                            // Seek back and update the passbook record
                            fseek(transaction_file, -sizeof(struct Passbook), SEEK_CUR);
                            fwrite(&passbook, sizeof(struct Passbook), 1, transaction_file);
                            break;
                        }
                    }

                    // Now update receiver's passbook
                    fseek(transaction_file, 0, SEEK_SET); // Reset to beginning of transaction file
                    while (fread(&passbook, sizeof(struct Passbook), 1, transaction_file) == 1) {
                        if (strcmp(passbook.username, to_username) == 0) {
                            // Create a new transaction record for receiver
                            struct Transaction receiver_transaction;
                            strcpy(receiver_transaction.type, "Transfer In");
                            receiver_transaction.amount = transfer_amount;
                            strcpy(receiver_transaction.date, "2024-10-19"); // Use actual date here
                            strcpy(receiver_transaction.from_username, username);
                            strcpy(receiver_transaction.to_username, to_username);

                            // Add the transaction to the receiver's passbook
                            passbook.transactions[passbook.num_transactions++] = receiver_transaction;

                            // Seek back and update the passbook record
                            fseek(transaction_file, -sizeof(struct Passbook), SEEK_CUR);
                            fwrite(&passbook, sizeof(struct Passbook), 1, transaction_file);
                            break;
                        }
                    }

                    fclose(transaction_file); // Close the transaction file
                } else {
                    // Insufficient balance for sender
                    break;
                }
            }
        }
    }

    // Unlock the mutex after updating the customer file
    pthread_mutex_unlock(&customer_file_mutex);

    fclose(file);

    // Step 6: Send success or failure message to the client
    int response = (sender_found && receiver_found) ? 1 : -1;
    send(client_sockfd, &response, sizeof(response), 0);

    // Step 7: Clean up and exit the thread
    close(client_sockfd);
    pthread_exit(NULL);
}


void *handle_withdrawal_application(void *arg) {
    struct WithdrawalRequest *request = (struct WithdrawalRequest *)arg;
    int client_sockfd = request->client_sockfd;

    // Step 1: Send "amount" prompt to the client
    char send_message[] = "amount";
    ssize_t num_bytes_sent = send(client_sockfd, send_message, sizeof(send_message), 0);
    if (num_bytes_sent == -1) {
        perror("send");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 2: Receive the withdrawal amount from the client
    int withdrawal_amount;
    ssize_t num_bytes_received = recv(client_sockfd, &withdrawal_amount, sizeof(withdrawal_amount), 0);
    if (num_bytes_received == -1) {
        perror("recv");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 3: Read the customers file and update the balance for the specified customer
    FILE *file = fopen("customers.txt", "r+b");
    if (file == NULL) {
        perror("fopen");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    struct Customer temp_customer;
    int found = 0;

    // Lock the mutex to ensure exclusive access to the customer file
    pthread_mutex_lock(&customer_file_mutex);

    // Search for the customer and check if withdrawal is possible
    while (fread(&temp_customer, sizeof(struct Customer), 1, file) == 1) {
        if (strcmp(temp_customer.username, request->username) == 0) {
            found = 1;

            // Check if sufficient balance is available
            if (temp_customer.balance >= withdrawal_amount) {
                temp_customer.balance -= withdrawal_amount; // Deduct the withdrawal amount

                // Seek back and update the record
                fseek(file, -sizeof(struct Customer), SEEK_CUR);
                fwrite(&temp_customer, sizeof(struct Customer), 1, file);

                // Now, update the customer's passbook with the withdrawal transaction
                FILE *transaction_file = fopen("transactions.txt", "r+b");
                if (transaction_file == NULL) {
                    perror("fopen transactions.txt");
                    fclose(file);
                    pthread_mutex_unlock(&customer_file_mutex);  // Unlock before exiting
                    close(client_sockfd);
                    pthread_exit(NULL);
                }

                struct Passbook passbook;
                int passbook_found = 0;

                // Search for the passbook of the customer
                while (fread(&passbook, sizeof(struct Passbook), 1, transaction_file) == 1) {
                    if (strcmp(passbook.username, request->username) == 0) {
                        passbook_found = 1;

                        // Create the withdrawal transaction
                        struct Transaction withdrawal_transaction;
                        strcpy(withdrawal_transaction.type, "Withdrawal");
                        withdrawal_transaction.amount = withdrawal_amount;
                        strcpy(withdrawal_transaction.date, "2024-10-19"); // You can set the actual date here
                        strcpy(withdrawal_transaction.from_username, request->username);
                        strcpy(withdrawal_transaction.to_username, request->username);

                        // Add the withdrawal transaction to the passbook
                        passbook.transactions[passbook.num_transactions++] = withdrawal_transaction;

                        // Seek back and update the passbook record
                        fseek(transaction_file, -sizeof(struct Passbook), SEEK_CUR);
                        fwrite(&passbook, sizeof(struct Passbook), 1, transaction_file);
                        break;
                    }
                }

                fclose(transaction_file);

                // Send success response to the client
                int response = 1; // Success
                send(client_sockfd, &response, sizeof(response), 0);
            } else {
                // Not enough funds
                int response = -1; // Failure
                send(client_sockfd, &response, sizeof(response), 0);
            }
            break;
        }
    }

    // Unlock the mutex after updating the customer file
    pthread_mutex_unlock(&customer_file_mutex);

    fclose(file);

    // If customer not found, send failure response
    if (!found) {
        int response = -1; // Failure
        send(client_sockfd, &response, sizeof(response), 0);
    }

    // Step 4: Clean up and exit the thread
    close(client_sockfd);
    pthread_exit(NULL);
}



void *getpassbook(void *arg) {
    struct PassbookRequest *request = (struct PassbookRequest *)arg;
    int client_sockfd = request->client_sockfd;
    char *username = request->username;

    // Lock the mutex before accessing the file
    pthread_mutex_lock(&customer_file_mutex);

    FILE *file = fopen("transactions.txt", "rb");
    if (file == NULL) {
        perror("fopen");
        pthread_mutex_unlock(&customer_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    struct Passbook passbook;
    int found = 0;

    // Iterate through the passbook entries in the file
    while (fread(&passbook, sizeof(struct Passbook), 1, file) == 1) {
        // Check if the username matches
        if (strcmp(passbook.username, username) == 0) {
            found = 1;

            // Send the passbook structure to the client
            ssize_t num_bytes_sent = send(client_sockfd, &passbook, sizeof(passbook), 0);
            if (num_bytes_sent == -1) {
                perror("send");
                // Close socket and exit thread if the send failed
                close(client_sockfd);
                pthread_mutex_unlock(&customer_file_mutex); // Unlock before exiting
                pthread_exit(NULL);
            }
            break;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&customer_file_mutex); // Unlock the mutex after the file operation is complete

    // If passbook not found, send an error message
    if (!found) {
        int response = -1;
        ssize_t num_bytes_sent = send(client_sockfd, &response, sizeof(response), 0);
        if (num_bytes_sent == -1) {
            perror("send error response");
        }
    }

    // Close the socket if no longer needed
    close(client_sockfd);
    pthread_exit(NULL);
}


void *handle_feedback_request(void *arg) {
    struct FeedbackRequest *request = (struct FeedbackRequest *)arg;
    int client_sockfd = request->client_sockfd;

    // Step 1: Send "feedback" prompt to the client
    char send_message[] = "feedback";
    ssize_t num_bytes_sent = send(client_sockfd, send_message, sizeof(send_message), 0);
    if (num_bytes_sent == -1) {
        perror("send");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 2: Receive feedback from the client
    struct Feedback feedback;
    strncpy(feedback.username, request->username, sizeof(feedback.username) - 1);
    feedback.username[sizeof(feedback.username) - 1] = '\0'; // Null-terminate

    // Receive the feedback
    ssize_t num_bytes_received = recv(client_sockfd, feedback.feedback, sizeof(feedback.feedback), 0);
    if (num_bytes_received == -1) {
        perror("recv");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Ensure null-termination
    feedback.feedback[sizeof(feedback.feedback) - 1] = '\0'; // Null-terminate
    // Optionally, truncate feedback if it's longer than 100 characters
    if (strlen(feedback.feedback) > 100) {
        feedback.feedback[100] = '\0';
    }

    // Step 3: Lock the mutex before accessing the feedback file
    pthread_mutex_lock(&feedback_file_mutex);

    // Write the structure to the feedbacks.txt file
    FILE *file = fopen("feedbacks.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        pthread_mutex_unlock(&feedback_file_mutex);  // Unlock before exiting
        close(client_sockfd);
        pthread_exit(NULL);
    }

    if (fwrite(&feedback, sizeof(struct Feedback), 1, file) != 1) {
        perror("fwrite");
        fclose(file);

        // Send failure response to the client
        int failure_message = -1;
        send(client_sockfd, &failure_message, sizeof(failure_message), 0);

        pthread_mutex_unlock(&feedback_file_mutex);  // Unlock before exiting
        close(client_sockfd);
        pthread_exit(NULL);
    }

    fclose(file);

    // Step 4: Send success message to the client
    int success_message = 1;
    ssize_t success_bytes_sent = send(client_sockfd, &success_message, sizeof(success_message), 0);
    if (success_bytes_sent == -1) {
        perror("send");
    }

    // Step 5: Unlock the mutex after the file operation is complete
    pthread_mutex_unlock(&feedback_file_mutex);

    // Step 6: Clean up and exit the thread
    close(client_sockfd);
    pthread_exit(NULL);
}

void *handle_list_pending_loan_applications(void *arg) {
    struct LoanRequest *request = (struct LoanRequest *)arg;
    int client_sockfd = request->client_sockfd;

    // Step 1: Open the loanApplications.txt file for reading
    FILE *file = fopen("loanApplications.txt", "rb");
    if (file == NULL) {
        perror("fopen");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    struct LoanApplication loan_application;
    int pending_count = 0;

    // Step 2: Count the number of pending applications
    while (fread(&loan_application, sizeof(struct LoanApplication), 1, file) == 1) {
        if (strcmp(loan_application.assigned_employee, "None") == 0) {
            pending_count++;
        }
    }

    // Step 3: Send the number of pending applications to the client
    ssize_t num_bytes_sent = send(client_sockfd, &pending_count, sizeof(pending_count), 0);
    if (num_bytes_sent == -1) {
        perror("send");
        fclose(file);
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 4: Reset the file pointer to the beginning of the file
    rewind(file);

    // Step 5: Send each pending application to the client
    while (fread(&loan_application, sizeof(struct LoanApplication), 1, file) == 1) {
        if (strcmp(loan_application.assigned_employee, "None") == 0) {
            // Send the application to the client
            num_bytes_sent = send(client_sockfd, &loan_application, sizeof(struct LoanApplication), 0);
            if (num_bytes_sent == -1) {
                perror("send");
                fclose(file);
                close(client_sockfd);
                pthread_exit(NULL);
            }

            // Step 6: Wait for acknowledgment from the client
            int ack;
            ssize_t num_bytes_received = recv(client_sockfd, &ack, sizeof(ack), 0);
            if (num_bytes_received == -1) {
                perror("recv");
                fclose(file);
                close(client_sockfd);
                pthread_exit(NULL);
            }

            // Optionally handle the acknowledgment
            if (ack != 1) {
                printf("Client did not acknowledge application for user %s\n", loan_application.username);
            }
        }
    }

    // Step 7: Close the file and clean up
    fclose(file);
    close(client_sockfd);
    pthread_exit(NULL);
}

// Function to handle listing employees with at least one None in processing_usernames
void *handle_pending_employees(void *arg) {
    struct LoanRequest *request = (struct LoanRequest *)arg;
    int client_sockfd = request->client_sockfd;

    FILE *file = fopen("employees.txt", "rb");
    if (file == NULL) {
        perror("fopen");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    struct Employee employee;
    int pending_count = 0;

    // First pass: Count employees with at least one "None"
    while (fread(&employee, sizeof(struct Employee), 1, file) == 1) {
        for (int i = 0; i < MAX_EMPLOYEES; i++) {
            if (strcmp(employee.processing_usernames[i], "None") == 0) {
                pending_count++;
                break; // No need to check other usernames for this employee
            }
        }
    }

    // Send the count of pending employees to the client
    send(client_sockfd, &pending_count, sizeof(pending_count), 0);
    
    // Reset file pointer to the beginning for the second pass
    fseek(file, 0, SEEK_SET);

    // Second pass: Send each employee with at least one "None" to the client
    while (fread(&employee, sizeof(struct Employee), 1, file) == 1) {
        for (int i = 0; i < MAX_EMPLOYEES; i++) {
            if (strcmp(employee.processing_usernames[i], "None") == 0) {
                // Send employee details to the client
                send(client_sockfd, &employee, sizeof(struct Employee), 0);
                int ack;
                recv(client_sockfd, &ack, sizeof(ack), 0); // Wait for acknowledgment
                break; // No need to check other usernames for this employee
            }
        }
    }

    fclose(file);
    close(client_sockfd);
    pthread_exit(NULL);
}

void *handle_assign_loan_to_employee(void *arg) {
    int client_sockfd = *(int *)arg;
    printf("Going to assign loan to employee\n");

    // Step 1: Ask the client for the loan applicant's username
    char keyword[] = "applicant";
    send(client_sockfd, keyword, sizeof(keyword), 0);
    
    char loan_applicant_username[20];
    recv(client_sockfd, loan_applicant_username, sizeof(loan_applicant_username), 0);
    printf("Received loan applicant username: %s\n", loan_applicant_username);

    FILE *loan_file = fopen("loanApplications.txt", "r+b");
    if (!loan_file) {
        perror("fopen loanApplications");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    struct LoanApplication loan_app;
    int loan_found = 0;

    // Check if the loan applicant exists and has a "None" employee assigned
    while (fread(&loan_app, sizeof(struct LoanApplication), 1, loan_file) == 1) {
        if (strcmp(loan_app.username, loan_applicant_username) == 0 && strcmp(loan_app.assigned_employee, "None") == 0) {
            loan_found = 1;
            break;
        }
    }

    if (!loan_found) {
        printf("Loan application for %s not found or already assigned.\n", loan_applicant_username);
        fclose(loan_file);
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 2: Ask the client for the employee's username
    strcpy(keyword, "employee");
    send(client_sockfd, keyword, sizeof(keyword), 0);

    char employee_username[20];
    recv(client_sockfd, employee_username, sizeof(employee_username), 0);
    printf("Received employee username: %s\n", employee_username);

    FILE *employee_file = fopen("employees.txt", "r+b");
    if (!employee_file) {
        perror("fopen employees");
        fclose(loan_file);
        close(client_sockfd);
        pthread_exit(NULL);
    }

    struct Employee employee;
    int employee_found = 0;
    int loan_assigned = 0;

    // Check if the employee exists and has space in the processing_usernames array
    while (fread(&employee, sizeof(struct Employee), 1, employee_file) == 1) {
        if (strcmp(employee.username, employee_username) == 0) {
            employee_found = 1;
            for (int i = 0; i < 5; i++) {
                if (strcmp(employee.processing_usernames[i], "None") == 0) {
                    // Assign the loan applicant to the employee
                    strncpy(loan_app.assigned_employee, employee_username, 20 - 1);
                    loan_app.assigned_employee[20 - 1] = '\0';

                    // Update the loan application
                    fseek(loan_file, -sizeof(struct LoanApplication), SEEK_CUR);
                    fwrite(&loan_app, sizeof(struct LoanApplication), 1, loan_file);
                    fflush(loan_file);

                    // Update the employee processing usernames
                    strncpy(employee.processing_usernames[i], loan_applicant_username, 20 - 1);
                    employee.processing_usernames[i][20 - 1] = '\0';

                    // Update the employee file
                    fseek(employee_file, -sizeof(struct Employee), SEEK_CUR);
                    fwrite(&employee, sizeof(struct Employee), 1, employee_file);
                    fflush(employee_file);

                    loan_assigned = 1;
                    break;
                }
            }
            break;
        }
    }

    fclose(loan_file);
    fclose(employee_file);

    // Step 3: Send result of the operation to the client
    if (loan_assigned) {
        char success_msg[] = "Loan successfully assigned!";
        send(client_sockfd, success_msg, sizeof(success_msg), 0);
    } else if (!employee_found) {
        char fail_msg[] = "Employee not found!";
        send(client_sockfd, fail_msg, sizeof(fail_msg), 0);
    } else {
        char fail_msg[] = "Employee cannot take more loans!";
        send(client_sockfd, fail_msg, sizeof(fail_msg), 0);
    }

    close(client_sockfd);
    pthread_exit(NULL);
}

void *handle_get_employee_loans(void *arg) {
    struct EmployeeLoanRequest *loan_request = (struct EmployeeLoanRequest *)arg;
    int client_sockfd = loan_request->client_sockfd;
    char employee_username[20];
    strcpy(employee_username, loan_request->employee_username); // Copy employee username
    free(loan_request); // Free the loan request structure

    // Step 2: Open employees.txt and loanApplications.txt
    FILE *employee_file = fopen("employees.txt", "rb");
    FILE *loan_file = fopen("loanApplications.txt", "rb");
    if (!employee_file || !loan_file) {
        perror("File open error");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 3: Find the employee in the employees.txt file
    struct Employee employee;
    int employee_found = 0;
    while (fread(&employee, sizeof(struct Employee), 1, employee_file) == 1) {
        if (strcmp(employee.username, employee_username) == 0) {
            employee_found = 1;
            break;
        }
    }

    if (!employee_found) {
        printf("Employee %s not found.\n", employee_username);
        close(client_sockfd);
        fclose(employee_file);
        fclose(loan_file);
        pthread_exit(NULL);
    }

    // Step 4: Count how many loans are assigned to this employee
    struct LoanApplication loan_app;
    int loan_count = 0;
    struct LoanApplication assigned_loans[5]; // Store up to 5 loans

    while (fread(&loan_app, sizeof(struct LoanApplication), 1, loan_file) == 1) {
        for (int i = 0; i < 5; i++) {
            if (strcmp(loan_app.assigned_employee, employee_username) == 0 &&
                strcmp(employee.processing_usernames[i], loan_app.username) == 0) {
                assigned_loans[loan_count++] = loan_app;
            }
        }
    }

    // Step 5: Send the number of loans back to the client
    send(client_sockfd, &loan_count, sizeof(loan_count), 0);

    // Step 6: Send each loan's details one by one
    for (int i = 0; i < loan_count; i++) {
        send(client_sockfd, &assigned_loans[i], sizeof(struct LoanApplication), 0);
    }

    // Close the files and socket
    fclose(employee_file);
    fclose(loan_file);
    close(client_sockfd);
    pthread_exit(NULL);
}

void *handle_loan_approval(void *arg) {
    struct EmployeeLoanRequest *loan_request = (struct EmployeeLoanRequest *)arg;
    int client_sockfd = loan_request->client_sockfd;
    char employee_username[20];
    strncpy(employee_username, loan_request->employee_username, sizeof(employee_username) - 1);
    employee_username[sizeof(employee_username) - 1] = '\0'; // Ensure null-termination
    free(loan_request);

    // Step 1: Send request for customer username
    if (send(client_sockfd, "username", sizeof("username"), 0) == -1) {
        perror("send error");
        close(client_sockfd);
        pthread_exit(NULL);
    }
    
    // Step 2: Receive the customer's username
    char customer_username[20];
    if (recv(client_sockfd, customer_username, sizeof(customer_username), 0) <= 0) {
        perror("recv error");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 3: Open loanApplications.txt to find the loan
    FILE *loan_file = fopen("loanApplications.txt", "rb+");
    if (!loan_file) {
        perror("File open error");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    struct LoanApplication loan_app;
    int loan_found = 0;

    // Step 4: Find the loan for the customer
    while (fread(&loan_app, sizeof(struct LoanApplication), 1, loan_file) == 1) {
        if (strcmp(loan_app.username, customer_username) == 0) {
            loan_found = 1;
            break;
        }
    }

    if (!loan_found) {
        printf("Loan for customer %s not found.\n", customer_username);
        fclose(loan_file);
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 5: Send loan details to client
    struct LoanApprovalRequest loan_request_info;
    strncpy(loan_request_info.username, loan_app.username, sizeof(loan_request_info.username) - 1);
    loan_request_info.username[sizeof(loan_request_info.username) - 1] = '\0';
    loan_request_info.amount = loan_app.amount;
    if (send(client_sockfd, &loan_request_info, sizeof(loan_request_info), 0) == -1) {
        perror("send error");
        fclose(loan_file);
        close(client_sockfd);
        pthread_exit(NULL);
    }
    
    // Step 6: Receive the response from the client
    char response[20];
    if (recv(client_sockfd, response, sizeof(response), 0) <= 0) {
        perror("recv error");
        fclose(loan_file);
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Step 7: Process response
    if (strcmp(response, "reject") == 0) {
        // Delete the loan application record (optional)
    } else if (strcmp(response, "approve") == 0) {
        // Step 8: Update the customer's balance directly in customers.txt
        FILE *customer_file = fopen("customers.txt", "rb+");
        if (!customer_file) {
            perror("File open error");
            fclose(loan_file);
            close(client_sockfd);
            pthread_exit(NULL);
        }

        struct Customer customer;
        int customer_found = 0;

        // Lock the mutex to ensure exclusive access to the customer file
        pthread_mutex_lock(&customer_file_mutex);

        // Find the customer and update balance
        while (fread(&customer, sizeof(struct Customer), 1, customer_file) == 1) {
            if (strcmp(customer.username, customer_username) == 0) {
                customer_found = 1;
                customer.balance += loan_app.amount; // Deposit the loan amount
                fseek(customer_file, -sizeof(struct Customer), SEEK_CUR);
                fwrite(&customer, sizeof(struct Customer), 1, customer_file);
                fflush(customer_file); // Ensure the data is written to the file
                break;
            }
        }

        if (!customer_found) {
            printf("Customer %s not found.\n", customer_username);
        }

        fclose(customer_file);

        // Step 9: Update passbook for the customer
        FILE *transaction_file = fopen("transactions.txt", "r+b");
        if (transaction_file == NULL) {
            perror("File open error for transactions.txt");
            fclose(loan_file);
            pthread_exit(NULL);
        }

        struct Passbook passbook;
        int passbook_found = 0;

        // Lock the mutex to ensure exclusive access to the passbook file
        pthread_mutex_lock(&customer_file_mutex);

        // Search for the passbook of the customer
        while (fread(&passbook, sizeof(struct Passbook), 1, transaction_file) == 1) {
            if (strcmp(passbook.username, customer_username) == 0) {
                passbook_found = 1;

                // Create the loan deposit transaction
                struct Transaction loan_transaction;
                strcpy(loan_transaction.type, "Loan Deposit");
                loan_transaction.amount = loan_app.amount;

                // Get the current date
                time_t now = time(NULL);
                strftime(loan_transaction.date, sizeof(loan_transaction.date), "%Y-%m-%d", localtime(&now));
                strcpy(loan_transaction.from_username, "cashFlow"); // Source of loan
                strcpy(loan_transaction.to_username, customer_username); // Destination is the customer

                // Add the loan transaction to the passbook
                if (passbook.num_transactions < 100) {
                    passbook.transactions[passbook.num_transactions++] = loan_transaction;

                    // Seek back and update the passbook record
                    fseek(transaction_file, -sizeof(struct Passbook), SEEK_CUR);
                    fwrite(&passbook, sizeof(struct Passbook), 1, transaction_file);
                    fflush(transaction_file); // Ensure the data is written to the file
                } else {
                    printf("Passbook for customer %s is full.\n", customer_username);
                }
                break;
            }
        }

        if (!passbook_found) {
            printf("Passbook for customer %s not found.\n", customer_username);
        }

        fclose(transaction_file);
        pthread_mutex_unlock(&customer_file_mutex); // Unlock the mutex after updating passbook
    }

    // Step 10: Update Employee structure
    FILE *employee_file = fopen("employees.txt", "rb+");
    if (!employee_file) {
        perror("File open error for employees.txt");
        fclose(loan_file);
        close(client_sockfd);
        pthread_exit(NULL);
    }

    struct Employee employee;
    int employee_found = 0;

    // Lock the mutex to ensure exclusive access to the employee file
    pthread_mutex_lock(&customer_file_mutex);

    // Find the employee and update processing_usernames
    while (fread(&employee, sizeof(struct Employee), 1, employee_file) == 1) {
        if (strcmp(employee.username, employee_username) == 0) {
            employee_found = 1;

            // Remove the customer username from processing_usernames
            for (int i = 0; i < 5; i++) {
                if (strcmp(employee.processing_usernames[i], customer_username) == 0) {
                    // Shift the remaining usernames
                    for (int j = i; j < 4; j++) {
                        strcpy(employee.processing_usernames[j], employee.processing_usernames[j + 1]);
                    }
                    // Clear the last username
                    memset(employee.processing_usernames[4], 0, sizeof(employee.processing_usernames[4]));
                    break;
                }
            }

            // Seek back and update the employee record
            fseek(employee_file, -sizeof(struct Employee), SEEK_CUR);
            fwrite(&employee, sizeof(struct Employee), 1, employee_file);
            fflush(employee_file); // Ensure the data is written to the file
            break;
        }
    }

    if (!employee_found) {
        printf("Employee %s not found.\n", employee_username);
    }

    fclose(employee_file);
    pthread_mutex_unlock(&customer_file_mutex); // Unlock the mutex after updating employee data

    // Step 11: Delete the loan application record
    FILE *temp_file = fopen("temp_loanApplications.txt", "wb");
    if (!temp_file) {
        perror("File open error");
        fclose(loan_file);
        close(client_sockfd);
        pthread_exit(NULL);
    }

    // Reset the file pointer to the beginning
    rewind(loan_file);

    // Write loans except the one that was approved
    while (fread(&loan_app, sizeof(struct LoanApplication), 1, loan_file) == 1) {
        if (strcmp(loan_app.username, customer_username) != 0) {
            fwrite(&loan_app, sizeof(struct LoanApplication), 1, temp_file);
        }
    }

    fclose(loan_file);
    fclose(temp_file);
    remove("loanApplications.txt");
    rename("temp_loanApplications.txt", "loanApplications.txt");

    // Close the socket
    close(client_sockfd);
    pthread_exit(NULL);
}




void *handle_client(void *arg) {
    int client_sockfd = *(int *)arg;
    free(arg);

    struct Operation operation;
    ssize_t num_bytes = recv(client_sockfd, &operation, sizeof(operation), 0);
    if (num_bytes == -1) {
        perror("recv");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    pthread_t thread_id;

    switch (operation.operation[0]) {
        case 'a': // Assuming 'a' is the prefix for add operations
            if (strcmp(operation.operation, "addcustomer") == 0) {
                struct Customer *customer_copy = malloc(sizeof(struct Customer));
                if (customer_copy == NULL) {
                    perror("malloc");
                    close(client_sockfd);
                    pthread_exit(NULL);
                }
                memcpy(customer_copy, &operation.data.customer, sizeof(struct Customer));
                pthread_create(&thread_id, NULL, addcustomer, customer_copy);

            } else if (strcmp(operation.operation, "addadmin") == 0) {
                struct Admin *admin_copy = malloc(sizeof(struct Admin));
                if (admin_copy == NULL) {
                    perror("malloc");
                    close(client_sockfd);
                    pthread_exit(NULL);
                }
                memcpy(admin_copy, &operation.data.admin, sizeof(struct Admin));
                pthread_create(&thread_id, NULL, addadmin, admin_copy);

            } else if (strcmp(operation.operation, "addemployee") == 0) {
                struct Employee *employee_copy = malloc(sizeof(struct Employee));
                if (employee_copy == NULL) {
                    perror("malloc");
                    close(client_sockfd);
                    pthread_exit(NULL);
                }
                memcpy(employee_copy, &operation.data.employee, sizeof(struct Employee));
                pthread_create(&thread_id, NULL, addemployee, employee_copy);

            } else if (strcmp(operation.operation, "addmanager") == 0) {
                struct Manager *manager_copy = malloc(sizeof(struct Manager));
                if (manager_copy == NULL) {
                    perror("malloc");
                    close(client_sockfd);
                    pthread_exit(NULL);
                }
                memcpy(manager_copy, &operation.data.manager, sizeof(struct Manager));
                pthread_create(&thread_id, NULL, addmanager, manager_copy);

            } 
            else if (strcmp(operation.operation, "assignLoan") == 0) {
            printf("Assigning loan to employee (switch)\n");

            // Allocate memory for the socket descriptor and pass it to the thread
            int *sockfd_ptr = malloc(sizeof(int));
            if (sockfd_ptr == NULL) {
                perror("malloc");
                close(client_sockfd);
                pthread_exit(NULL);
            }

            *sockfd_ptr = client_sockfd;
            pthread_t thread_id;

            // Create a new thread to handle loan assignment
            if (pthread_create(&thread_id, NULL, handle_assign_loan_to_employee, sockfd_ptr) != 0) {
                perror("pthread_create");
                free(sockfd_ptr); // Free the allocated memory if thread creation fails
                close(client_sockfd);
                pthread_exit(NULL);
            }

            // Optionally detach the thread to avoid resource leakage
            pthread_detach(thread_id);
        }
        else {
            printf("Unknown operation. Closing connection.\n");
            close(client_sockfd);
            pthread_exit(NULL);
        }
        break;


        case 'g':
            if (strcmp(operation.operation, "getbalance") == 0) {
            struct BalanceRequest *balance_request = malloc(sizeof(struct BalanceRequest));
            if (balance_request == NULL) {
                perror("malloc");
                close(client_sockfd);
                pthread_exit(NULL);
            }

            memcpy(&balance_request->customer, &operation.data.customer, sizeof(struct Customer));
            balance_request->client_sockfd = client_sockfd;

            printf("client_sockfd: %d\n", client_sockfd);

            // Create the thread and check for success
            if (pthread_create(&thread_id, NULL, getbalance, balance_request) != 0) {
                perror("pthread_create");
                free(balance_request);  // Free allocated memory on failure
                close(client_sockfd);
                pthread_exit(NULL);
            }
        }
else if (strcmp(operation.operation, "getEmployeeLoans") == 0) {
                // Create a structure to hold employee data and socket information
                struct EmployeeLoanRequest *loan_request = malloc(sizeof(struct EmployeeLoanRequest));
                if (loan_request == NULL) {
                    perror("malloc");
                    close(client_sockfd);
                    pthread_exit(NULL);
                }

                // Copy the employee username from the operation data
                memcpy(loan_request->employee_username, operation.data.customer.username, sizeof(operation.data.customer.username));
                loan_request->client_sockfd = client_sockfd;

                // Create a thread to handle fetching the employee's assigned loans
                pthread_create(&thread_id, NULL, handle_get_employee_loans, loan_request);
                pthread_detach(thread_id); // Detach the thread to handle it independently
            } else {
                close(client_sockfd);
                pthread_exit(NULL);
            }
            break;

        case 'l': // New case for loan application
            if (strcmp(operation.operation, "loanApplication") == 0) {
                struct LoanRequest *loan_request = malloc(sizeof(struct LoanRequest));
                if (loan_request == NULL) {
                    perror("malloc");
                    close(client_sockfd);
                    pthread_exit(NULL);
                }
                memcpy(&loan_request->username, &operation.data.customer.username, sizeof(struct Customer));
                loan_request->client_sockfd = client_sockfd;
                pthread_create(&thread_id, NULL, handle_loan_application, loan_request);
            } else if (strcmp(operation.operation, "loanApproval") == 0) {
                    struct LoanRequest *loan_approval_request = malloc(sizeof(struct LoanRequest));
                    if (loan_approval_request == NULL) {
                        perror("malloc");
                        close(client_sockfd);
                        pthread_exit(NULL);
                    }

                    // Copy the loan approval details from the operation data
                    memcpy(&loan_approval_request->username, &operation.data.customer.username, sizeof(loan_approval_request->username));
                    loan_approval_request->client_sockfd = client_sockfd; // Pass the socket descriptor

                    // Create a thread to handle loan approval
                    pthread_create(&thread_id, NULL, handle_loan_approval, loan_approval_request);
                    pthread_detach(thread_id); // Detach the thread to handle it independently
                } else {
                    close(client_sockfd);
                    pthread_exit(NULL);
                }
                break;
    
    // Add this case inside the handle_client() switch statement
        case 'd': // New case for deposit operation
            if (strcmp(operation.operation, "depositMoney") == 0) {
                struct DepositRequest *deposit_request = malloc(sizeof(struct DepositRequest));
                if (deposit_request == NULL) {
                    perror("malloc");
                    close(client_sockfd);
                    pthread_exit(NULL);
                }

                // Copy the username and socket descriptor to the deposit request structure
                strncpy(deposit_request->username, operation.data.customer.username, sizeof(deposit_request->username) - 1);
                deposit_request->username[sizeof(deposit_request->username) - 1] = '\0'; // Null-terminate
                deposit_request->client_sockfd = client_sockfd;

                // Create a new thread to handle the deposit
                pthread_create(&thread_id, NULL, handle_deposit_application, deposit_request);
            } else {
                close(client_sockfd);
                pthread_exit(NULL);
            }
            break;

        // Add this case inside the handle_client() switch statement
        case 't': // New case for transfer operation
            if (strcmp(operation.operation, "transferMoney") == 0) {
                struct TransferRequest *transfer_request = malloc(sizeof(struct TransferRequest));
                if (transfer_request == NULL) {
                    perror("malloc");
                    close(client_sockfd);
                    pthread_exit(NULL);
                }

                // Copy the username and socket descriptor to the transfer request structure
                strncpy(transfer_request->username, operation.data.customer.username, sizeof(transfer_request->username) - 1);
                transfer_request->username[sizeof(transfer_request->username) - 1] = '\0'; // Null-terminate
                transfer_request->client_sockfd = client_sockfd;

                // Create a new thread to handle the transfer
                pthread_create(&thread_id, NULL, handle_transfer_money, transfer_request);
            } else {
                close(client_sockfd);
                pthread_exit(NULL);
            }
            break;
        
       case 'w': // New case for withdrawal operation
    if (strcmp(operation.operation, "withdrawMoney") == 0) {
        struct WithdrawalRequest *withdrawal_request = malloc(sizeof(struct WithdrawalRequest));
        if (withdrawal_request == NULL) {
            perror("malloc");
            close(client_sockfd);
            pthread_exit(NULL);
        }
        // Copy the username and socket descriptor to the withdrawal request structure
        strncpy(withdrawal_request->username, operation.data.customer.username, sizeof(withdrawal_request->username) - 1);
        withdrawal_request->username[sizeof(withdrawal_request->username) - 1] = '\0'; // Null-terminate
        withdrawal_request->client_sockfd = client_sockfd;
        // Create a new thread to handle the withdrawal
        pthread_create(&thread_id, NULL, handle_withdrawal_application, withdrawal_request);
    } else {
        close(client_sockfd);
        pthread_exit(NULL);
    }
    break;

    case 'v': // New case for getting passbook
    if (strcmp(operation.operation, "viewHistory") == 0) {
        struct PassbookRequest *passbook_request = malloc(sizeof(struct PassbookRequest));
        if (passbook_request == NULL) {
            perror("malloc");
            close(client_sockfd);
            pthread_exit(NULL);
        }

        // Copy the username and socket descriptor to the passbook request structure
        strncpy(passbook_request->username, operation.data.customer.username, sizeof(passbook_request->username) - 1);
        passbook_request->username[sizeof(passbook_request->username) - 1] = '\0'; // Null-terminate
        passbook_request->client_sockfd = client_sockfd;

        // Create a new thread to handle fetching the passbook
        pthread_create(&thread_id, NULL, getpassbook, passbook_request);
    } else {
        close(client_sockfd);
        pthread_exit(NULL);
    }

    break;

    case 'u': // New case for update username operation
        if (strcmp(operation.operation, "updateUsername") == 0) {
            struct PassbookRequest *passbook_request = malloc(sizeof(struct PassbookRequest));
            if (passbook_request == NULL) {

                perror("malloc");
                close(client_sockfd);
                pthread_exit(NULL);
            }

            // Copy the username and socket descriptor to the passbook request structure
            strncpy(passbook_request->username, operation.data.customer.username, sizeof(passbook_request->username) - 1);
            passbook_request->username[sizeof(passbook_request->username) - 1] = '\0'; // Null-terminate
            passbook_request->client_sockfd = client_sockfd;

            // Create a new thread to handle the username update
            pthread_create(&thread_id, NULL, update_username, passbook_request);
        } else {
            close(client_sockfd);
            pthread_exit(NULL);
        }
    break;

case 'f': // New case for feedback submission
    if (strcmp(operation.operation, "feedbackSubmission") == 0) {
        struct FeedbackRequest *feedback_request = malloc(sizeof(struct FeedbackRequest));
        if (feedback_request == NULL) {
            perror("malloc");
            close(client_sockfd);
            pthread_exit(NULL);
        }

        // Copy username from the operation data
        memcpy(feedback_request->username, operation.data.customer.username, sizeof(feedback_request->username));
        feedback_request->client_sockfd = client_sockfd;

        // Create a thread to handle feedback submission
        pthread_create(&thread_id, NULL, handle_feedback_request, feedback_request);
    } else {
        close(client_sockfd);
        pthread_exit(NULL);
    }
    break;

    case 'p': // Case for listing pending applications and pending employees
    if (strcmp(operation.operation, "pending") == 0) {
        struct LoanRequest *loan_request = malloc(sizeof(struct LoanRequest));
        if (loan_request == NULL) {
            perror("malloc");
            close(client_sockfd);
            pthread_exit(NULL);
        }

        memcpy(&loan_request->username, &operation.data.customer.username, sizeof(struct Customer));
        loan_request->client_sockfd = client_sockfd;

        // Create a thread to handle listing pending loan applications
        pthread_create(&thread_id, NULL, handle_list_pending_loan_applications, loan_request);
    } 
    else if (strcmp(operation.operation, "pendingEmployees") == 0) {
        struct EmployeeRequest *employee_request = malloc(sizeof(struct EmployeeRequest));
        if (employee_request == NULL) {
            perror("malloc");
            close(client_sockfd);
            pthread_exit(NULL);
        }

        // No specific user data is needed for this operation, just pass the socket
        employee_request->client_sockfd = client_sockfd;

        // Create a thread to handle listing employees with at least one "None"
        pthread_create(&thread_id, NULL, handle_pending_employees, employee_request);
    }
    break; // End of pending applications and employees case




        default:
            close(client_sockfd);
            pthread_exit(NULL);
    }

    pthread_detach(thread_id);
    pthread_exit(NULL);
}


int main() {
    int sockfd;
    struct sockaddr_un server_addr;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    unlink(SOCKET_PATH);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    if (listen(sockfd, 5) == -1) {
        perror("listen");
        close(sockfd);
        exit(1);
    }

    while (1) {
        int *client_sockfd = malloc(sizeof(int));
        if (client_sockfd == NULL) {
            perror("malloc");
            continue;
        }

        *client_sockfd = accept(sockfd, NULL, NULL);
        if (*client_sockfd == -1) {
            perror("accept");
            free(client_sockfd);
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, client_sockfd);
        pthread_detach(thread_id);
    }

    close(sockfd);
    unlink(SOCKET_PATH);
    return 0;
}