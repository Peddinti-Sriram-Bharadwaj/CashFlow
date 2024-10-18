#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/server"
#define BUFFER_SIZE 256

struct Customer{
    char username[20];
    int balance;
};

void handle_client(int sockfd, struct sockaddr_un client_addr, socklen_t client_addr_len, struct Customer *customer){
    printf("Received customer data from client: Username = %s, Balance = %d\n", customer->username, customer->balance);

    FILE *file = fopen("customers.txt", "ab");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }

    if (fwrite(customer, sizeof(struct Customer), 1, file) != 1) {
        perror("fwrite");
        fclose(file);
        exit(1);
    }
    fclose(file);
}

int main(){
    int sockfd;
    struct sockaddr_un server_addr, client_addr;
    struct Customer customer;
    socklen_t client_addr_len = sizeof(client_addr);

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    unlink(SOCKET_PATH);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        exit(1);
    }

    printf("Server is waiting for messages from clients...\n");

    while(1){
        ssize_t num_bytes = recvfrom(sockfd, &customer, sizeof(customer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if(num_bytes == -1){
            perror("recvfrom");
            exit(1);
        }

        handle_client(sockfd, client_addr, client_addr_len, &customer);
    }

    close(sockfd);
    unlink(SOCKET_PATH);
    return 0;
}