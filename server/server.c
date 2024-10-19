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

struct Employee {
    char username[20];
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
    struct Customer *customer = &request->customer;
    int client_sockfd = request->client_sockfd;

    // Lock the mutex before accessing the file
    pthread_mutex_lock(&customer_file_mutex);

    FILE *file = fopen("customers.txt", "rb");
    if (file == NULL) {
        perror("fopen");
        pthread_mutex_unlock(&customer_file_mutex);  // Unlock before exiting
        pthread_exit(NULL);
    }

    struct Customer temp_customer;
    int found = 0;

    // Search for the customer in the file
    while (fread(&temp_customer, sizeof(struct Customer), 1, file) == 1) {
        if (strcmp(temp_customer.username, customer->username) == 0) {
            found = 1;

            ssize_t num_bytes_sent = send(client_sockfd, &temp_customer.balance, sizeof(temp_customer.balance), 0);
            if (num_bytes_sent == -1) {
                perror("send");
            }
            break;
        }
    }
    fclose(file);

    // Unlock the mutex after the file operation is complete
    pthread_mutex_unlock(&customer_file_mutex);

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

    // Search for the customer and update their balance
    while (fread(&temp_customer, sizeof(struct Customer), 1, file) == 1) {
        if (strcmp(temp_customer.username, request->username) == 0) {
            found = 1;
            temp_customer.balance += deposit_amount;

            // Seek back and update the record
            fseek(file, -sizeof(struct Customer), SEEK_CUR);
            fwrite(&temp_customer, sizeof(struct Customer), 1, file);

            // Now, update the customer's passbook with the deposit transaction
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

                    // Create the deposit transaction
                    struct Transaction deposit_transaction;
                    strcpy(deposit_transaction.type, "Deposit");
                    deposit_transaction.amount = deposit_amount;
                    strcpy(deposit_transaction.date, "2024-10-19"); // You can set the actual date here
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
            }
            break;
        }
    }

    fclose(file);

    // Unlock the mutex after the file operation is complete
    pthread_mutex_unlock(&customer_file_mutex);

    // If passbook not found, send an error message
    if (!found) {
        // Optionally send an error response to the client
        int response = -1;
        ssize_t num_bytes_sent = send(client_sockfd, &response, sizeof(response), 0);
        if (num_bytes_sent == -1) {
            perror("send error response");
        }
    }

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

            } else {
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

                pthread_create(&thread_id, NULL, getbalance, balance_request);
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

// The rest of your server code continues here...



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