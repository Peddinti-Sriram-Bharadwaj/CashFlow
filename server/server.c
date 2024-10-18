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


// Function to handle adding a customer
void *addcustomer(void *arg) {
    struct Customer *customer = (struct Customer *)arg;

    FILE *file = fopen("customers.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        pthread_exit(NULL);
    }

    if (fwrite(customer, sizeof(struct Customer), 1, file) != 1) {
        perror("fwrite");
        fclose(file);
        pthread_exit(NULL);
    }
    fclose(file);
    pthread_exit(NULL);
}

// Function to handle adding an admin
void *addadmin(void *arg) {
    struct Admin *admin = (struct Admin *)arg;

    FILE *file = fopen("admins.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        pthread_exit(NULL);
    }

    if (fwrite(admin, sizeof(struct Admin), 1, file) != 1) {
        perror("fwrite");
        fclose(file);
        pthread_exit(NULL);
    }
    fclose(file);
    pthread_exit(NULL);
}

// Function to handle adding an employee
void *addemployee(void *arg) {
    struct Employee *employee = (struct Employee *)arg;

    FILE *file = fopen("employees.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        pthread_exit(NULL);
    }

    if (fwrite(employee, sizeof(struct Employee), 1, file) != 1) {
        perror("fwrite");
        fclose(file);
        pthread_exit(NULL);
    }
    fclose(file);
    pthread_exit(NULL);
}

// Function to handle adding a manager
void *addmanager(void *arg) {
    struct Manager *manager = (struct Manager *)arg;

    FILE *file = fopen("managers.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        pthread_exit(NULL);
    }

    if (fwrite(manager, sizeof(struct Manager), 1, file) != 1) {
        perror("fwrite");
        fclose(file);
        pthread_exit(NULL);
    }
    fclose(file);
    pthread_exit(NULL);
}

void *getbalance(void *arg) {
    struct BalanceRequest *request = (struct BalanceRequest *)arg;
    struct Customer *customer = &request->customer;
    int client_sockfd = request->client_sockfd;

    FILE *file = fopen("customers.txt", "rb");
    if (file == NULL) {
        perror("fopen");
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
    pthread_exit(NULL);
}

// Function to handle loan application
void *handle_loan_application(void *arg) {
    struct LoanRequest *request = (struct LoanRequest *)arg;
    int client_sockfd = request->client_sockfd;

    // Step 1: Send "send" to the client
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

    // Step 4: Write the structure to the loanApplications.txt file
    FILE *file = fopen("loanApplications.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        close(client_sockfd);
        pthread_exit(NULL);
    }

    if (fwrite(&loan_application, sizeof(struct LoanApplication), 1, file) != 1) {
        perror("fwrite");
        fclose(file);

        // Send failure response to the client
        int failure_message = -1;
        send(client_sockfd, &failure_message, sizeof(failure_message), 0);

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

    // Step 6: Clean up and exit the thread
    close(client_sockfd);
    pthread_exit(NULL);
}

// Function to handle deposit application
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

    // Search for the customer and update their balance
    while (fread(&temp_customer, sizeof(struct Customer), 1, file) == 1) {
        if (strcmp(temp_customer.username, request->username) == 0) {
            found = 1;
            temp_customer.balance += deposit_amount;

            // Seek back and update the record
            fseek(file, -sizeof(struct Customer), SEEK_CUR);
            fwrite(&temp_customer, sizeof(struct Customer), 1, file);
            break;
        }
    }

    fclose(file);

    // Step 4: Send success or failure message to the client
    int response = found ? 1 : -1;
    send(client_sockfd, &response, sizeof(response), 0);

    // Step 5: Clean up and exit the thread
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
