#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

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

// Function to handle adding a customer
void *addcustomer(void *arg) {
    struct Customer *customer = (struct Customer *)arg;
    printf("Received customer data: Username = %s, Balance = %d\n", customer->username, customer->balance);

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
    printf("Received admin data: Username = %s\n", admin->username);

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
    printf("Received employee data: Username = %s\n", employee->username);

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
    printf("Received manager data: Username = %s\n", manager->username);

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

int main() {
    int sockfd;
    struct sockaddr_un server_addr, client_addr;
    struct Operation operation;
    socklen_t client_addr_len = sizeof(client_addr);

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;

    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    unlink(SOCKET_PATH); // Remove the socket file if it exists
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    printf("Server is waiting for messages from clients...\n");

    while (1) {
        ssize_t num_bytes = recvfrom(sockfd, &operation, sizeof(operation), 0,
                                      (struct sockaddr *)&client_addr, &client_addr_len);

        if (num_bytes == -1) {
            perror("recvfrom");
            exit(1);
        }

        // Create a thread for each operation
        pthread_t thread_id;

        switch (operation.operation[0]) { // Using the first character to determine operation type
            case 'a': // Assuming 'a' is the prefix for add operations
                if (strcmp(operation.operation, "addcustomer") == 0) {
                    struct Customer *customer_copy = malloc(sizeof(struct Customer));
                    memcpy(customer_copy, &operation.data.customer, sizeof(struct Customer));
                    pthread_create(&thread_id, NULL, addcustomer, customer_copy);

                } else if (strcmp(operation.operation, "addadmin") == 0) {
                    struct Admin *admin_copy = malloc(sizeof(struct Admin));
                    memcpy(admin_copy, &operation.data.admin, sizeof(struct Admin));
                    pthread_create(&thread_id, NULL, addadmin, admin_copy);

                } else if (strcmp(operation.operation, "addemployee") == 0) {
                    struct Employee *employee_copy = malloc(sizeof(struct Employee));
                    memcpy(employee_copy, &operation.data.employee, sizeof(struct Employee));
                    pthread_create(&thread_id, NULL, addemployee, employee_copy);

                } else if (strcmp(operation.operation, "addmanager") == 0) {
                    struct Manager *manager_copy = malloc(sizeof(struct Manager));
                    memcpy(manager_copy, &operation.data.manager, sizeof(struct Manager));
                    pthread_create(&thread_id, NULL, addmanager, manager_copy);

                } else {
                    printf("Unknown operation: %s\n", operation.operation);
                    continue; // Skip to the next iteration for unknown operations
                }
                break;

            default:
                printf("Unknown operation prefix: %s\n", operation.operation);
                continue; // Skip to the next iteration for unknown prefixes
        }

        // Detach the thread so that it cleans up after itself when done
        pthread_detach(thread_id);
        
     }

     close(sockfd);
     unlink(SOCKET_PATH); // Clean up the socket file on exit
     return 0;
}