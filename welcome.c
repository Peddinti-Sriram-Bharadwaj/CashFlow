#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void* run_cashflow_service(void* arg) {
    // Get the thread ID
    pthread_t thread_id = pthread_self();
    printf("Thread ID: %lu\n", (unsigned long)thread_id); // Print the thread ID

    printf("\n========================================\n");
    printf("  Welcome to CashFlow Banking Services\n");
    printf("========================================\n");
    printf("Please choose one of the following options to continue:\n");
    printf("  1. Admin\n");
    printf("  2. Manager\n");
    printf("  3. Employee\n");
    printf("  4. Customer\n");
    printf("  5. Exit\n");
    printf("========================================\n");

    int option;
    scanf("%d", &option);

    switch (option) {
        case 1:
            printf("Admin\n");
            execl("./admin/login.out", "login.out", NULL);
            perror("execl failed"); // If execl fails
            break;
        case 2:
            printf("Manager\n");
            execl("./manager/login.out", "login.out", NULL);
            perror("execl failed"); // If execl fails
            break;
        case 3:
            printf("Employee\n");
            execl("./employee/login.out", "login.out", NULL);
            perror("execl failed"); // If execl fails
            break;
        case 4:
            printf("Customer\n");
            execl("./customer/login.out", "login.out", NULL);
            perror("execl failed"); // If execl fails
            break;
        case 5:
            printf("Exiting CashFlow services\n");
            break;
        default:
            printf("Invalid option\n");
            break;
    }

    return NULL; // Return from the thread function
}

int main() {
    pthread_t thread;

    // Create a new thread to run the CashFlow service
    if (pthread_create(&thread, NULL, run_cashflow_service, NULL) != 0) {
        perror("Failed to create thread");
        return 1; // Exit with error
    }

    // Wait for the created thread to finish
    pthread_join(thread, NULL);

    return 0;
}